ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; The BPB (BIOS Parameter Block) is required by some computer BIOS.
; The first 3 bytes always consist of a short jump followed by a NOP instruction.
_start:
    jmp short start
    nop
; Set the rest of the BPB parameters to 0.
times 33 db 0

start:
    ; It is important that the code segment is changed to 0x00 because the origin is 0x7c00.
    jmp 0:init

init:
    ; Ensure that the segment registers are properly set to avoid problems if the BIOS does not set them correctly.
    ; Since our origin is 0x7c00, all our segment registers should initially be zero.
    ; Clear interrupts because we do not want hardware interrupts to occur during the critical operation
    ; of configuring the segment registers.
    cli
    ; We cannot put 0x00 directly into DS and ES.
    ; We need to first load the value into the AX register, this is required by the way the processor works.
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    ; Set the stack segment to 0x00 (the default) and initialize the stack pointer to 0x7c00.
    mov ss, ax
    mov sp, 0x7c00
    ; Enable interrupts.
    sti

.load_protected:
    cli
    ; Load the GDT.
    lgdt [gdt_descriptor]
    ; Get the current configuration in the CR0 register.
    mov eax, cr0
    ; Set the first bit of CR0 to enable protected mode.
    or eax, 0x1
    ; Store the new configuration in the CR0 register.
    mov cr0, eax
    ; Perform a long jump to ensure that prefetching is cleared, and the next instruction is executed in protected mode.
    ; Also, set the CS register to use the CODE_SEG entry of the GDT.
    jmp CODE_SEG:load32

; GDT
gdt_start:
; GDT offset 0 (entry 0).
gdt_null:
    dd 0x0
    dd 0x0

; CS should point to this GDT entry.
; Lim 0xFFFFF
; Base 0x00000000
; Acc 0x9A (1001 1010)
; - Readable code segment
; - Executable segment
; Flg 0xC (1100)
; GDT offset 0x08 (entry 1).
gdt_code:
    ; 0-15: Limit low
    dw 0xffff
    ; 16-31: Base low
    dw 0
    ; 32-39: Base middle
    db 0
    ; 40-47: Access byte
    db 0x9a
    ; 48-55: Limit high and flags
    db 11001111b
    ; 56-63: Base high
    db 0

; DS, SS, ES, FS, and GS should point to this GDT entry.
; Lim 0xFFFFF
; Base 0x00000000
; Acc 0x92 (1001 0010)
; - Writable data segment
; Flg 0xC (1100)
; GDT offset 0x10 (entry 2).
gdt_data:
    ; 0-15: Limit low
    dw 0xffff
    ; 16-31: Base low
    dw 0
    ; 32-39: Base middle
    db 0
    ; 40-47: Access byte
    db 0x92
    ; 48-55: Limit high and flags
    db 11001111b
    ; 56-63: Base high
    db 0

gdt_end:

; Provide the size and starting address of the GDT.
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    ; Enable the A20 line.
    in al, 0x92
    or al, 2
    out 0x92, al

    mov ax, DATA_SEG
    ; Set up the DS, SS, ES, FS, and GS segments.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; Set up the stack.
    mov ss, ax
    mov ebp, 0x00400000
    mov esp, ebp

    ; Load the kernel.
    ; Start reading at sector 2.
    mov eax, 1
    ; 512 * 192 = 98,304 bytes to load.
    mov ecx, 192
    ; The address in memory to load the sectors.
    mov edi, 0x00100000
    call ata_lba_read

    ; Perform a long jump to the kernel.
    ; This ensures that the CS register becomes the code selector specified in the GDT.
    jmp CODE_SEG:0x00100000

; eax: Starting sector.
; ecx: Number of sectors to read.
; edi: Address to store the read data.
ata_lba_read:
    ; Save the value of EAX (logical block address - LBA) into EBX for later use.
    mov ebx, eax
    ; Shift the value in EAX 24 bits to the right (equivalent to EAX >> 24).
    ; This isolates the highest byte of the LBA.
    shr eax, 24
    ; Set the upper 4 bits of AL to 0b1110 (master disk selection).
    ; Combine the drive/head select byte with the highest LBA byte.
    or eax, 0xe0
    ; Write the drive/head byte (AL) to port 0x1F6 (Drive/Head register).
    mov dx, 0x1f6
    out dx, al

    ; Write the number of sectors to read (stored in ECX) to port 0x1F2 (Sector Count register).
    mov eax, ecx
    mov dx, 0x1f2
    out dx, al

    ; Write the first (lowest) 8 bits of the LBA (stored in EBX) to port 0x1F3 (LBA low register).
    mov eax, ebx
    mov dx, 0x1f3
    out dx, al

    ; Write the next 8 bits (bits 8-15) of the LBA to port 0x1F4 (LBA mid register).
    mov dx, 0x1f4
    mov eax, ebx
    shr eax, 8
    out dx, al

    ; Write the next 8 bits (bits 16-23) of the LBA to port 0x1F5 (LBA high register).
    mov dx, 0x1f5
    mov eax, ebx
    shr eax, 16
    out dx, al

    ; Send the "read sectors" command (0x20) to port 0x1F7 (Command register).
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.next_sector:
    ; Save the current value of ECX onto the stack to restore it later.
    push ecx

.try_again:
    ; Load the status port address (0x1F7) into DX.
    mov dx, 0x1f7
    ; Read the status of the disk from port 0x1F7 and store the byte in AL.
    in al, dx
    ; Check if bit 3 of AL is set (indicating that the disk is ready to provide data).
    test al, 8
    ; If bit 3 is not set (test result is zero), jump back to .try_again to retry.
    ; This creates a wait loop until the disk is ready.
    jz .try_again
    ; Initialize ECX to 256 (the number of words to read, i.e., 512 bytes, which is one sector).
    mov ecx, 256

    ; Load the data port address (0x1F0) into DX.
    mov dx, 0x1f0

    ; Repeat the INSW instruction while ECX is not zero:
    ; - INSW reads one word (16 bits) from the port specified by DX and stores it at the location pointed to by ES:EDI.
    ; - ECX is decremented on each iteration.
    ; This transfers 512 bytes (one sector) from the disk to memory.
    rep insw

    ; Restore the previously saved value of ECX from the stack.
    pop ecx

    ; Decrement ECX.
    ; If ECX is not zero, jump to the .next_sector label, otherwise, exit the loop.
    loop .next_sector
    ret


; Fill remaining bytes up to 510 with 0.
times 510 - ($ - $$) db 0
; Boot signature.
dw 0xAA55