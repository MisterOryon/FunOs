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
    mov ax, DATA_SEG
    ; Set up the DS, SS, ES, FS, and GS segments.
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ; Set up the stack.
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp
    ; Infinite loop to stop further execution.
    jmp $

; Fill remaining bytes up to 510 with 0.
times 510 - ($ - $$) db 0
; Boot signature.
dw 0xAA55