ORG 0

; Tell the assembler that we are using 16-bit architecture.
BITS 16

; The BPB (BIOS Parameter Block) is required by some computer BIOS.
; The first 3 bytes always consist of a short jump followed by a NOP instruction.
_start:
    jmp short start
    nop
; Set the rest of the BPB parameters to 0.
times 33 db 0

start:
    ; Make the start segment become 0x7c0 because the origin is 0.
    ; It is important that the code segment is also changed to 0x7c0.
    jmp 0x7c0:init

init:
    ; Ensure that segment registers are correctly set to avoid problems if the BIOS does not set DS and ES to 0x7c0.
    ; Clear interrupts because we do not want hardware interrupts to occur during the critical operation
    ; of configuring the segment registers.
    cli
    ; We cannot put 0x7c0 directly into DS and ES.
    ; We need to first load the value into the AX register ,this is required by the way the processor works.
    mov ax, 0x7c0
    mov ds, ax
    mov es, ax
    ; Set the stack segment to 0x00 (the default) and initialize the stack pointer to 0x7c00.
    mov ax, 0x00
    mov ss, ax
    mov sp, 0x7c00
    ; Enable interrupts.
    sti

    call setup_screen
    call clean_screen

    ; Set up interrupt 0x80 to print the string pointed to by the SI register.
    ; Set the segment of interrupt 0x80 with our code segment address.
    ; Since the SS segment is equal to zero, the absolute address is calculated as 0x00 * 16 + 0x202 = 0x202.
    mov ax, 0x7c0
    mov word[ss:0x202], ax
    ; Calculate the offset of 'print' and set the offset of interrupt 0x80 with the 'print' offset.
    lea ax, [print]
    mov word[ss:0x200], ax

    ; Move the address of the message label into the SI register.
    mov si, message
    ; Call our print interrupt
    int 0x80

    ; Jump to itself to ensure we don't execute unwanted instructions beyond the signature.
    jmp $

setup_screen:
    ; Ask the BIOS to switch to the video mode that supports an 80x25 screen.
    ; Int 10/AH=00h (VIDEO - SET VIDEO MODE).
    mov ah, 00h
    mov al, 03h
    int 0x10

    ; Ask the BIOS to select display page one.
    ; Int 10/AH=05h (VIDEO - SELECT ACTIVE DISPLAY PAGE).
    mov ah, 05h
    mov al, 00h
    int 0x10

    ret

clean_screen:
    ; Ask the BIOS to scroll up the screen to clear its contents.
    ; Int 10/AH=06h (VIDEO - SCROLL UP WINDOW).
    mov ah, 06h
    ; Clear the entire screen.
    mov al, 00h
    ; Set white text color on a black background.
    mov bh, 0x07
    int 0x10

    ; Ask the BIOS to reset the cursor position to 0,0 on page one.
    ; Int 10/AH=02h (VIDEO - SET CURSOR POSITION).
    mov ah, 02h
    ; Page one.
    mov bh, 00h
    ; Row position.
    mov dh, 00h
    ; Column position.
    mov dl, 00h
    int 0x10

    ret

print:
.loop:
    ; Load the character that the SI register is pointing to into the AL register;
    ; then increment the SI register so it points to the next character.
    lodsb
    ; Compare AL to 0.
    cmp al, 0
    ; If equal, jump to done (JE for jump if equal).
    je .done
    ; Otherwise, call print_char.
    call print_char
    ; Loop again to display the next character.
    jmp .loop
.done:
    ; Return from interrupt routine ISR.
    iret

print_char:
    ; Ask the BIOS to display a character on the screen.
    ; INT 10/AH=0Eh (VIDEO - TELETYPE OUTPUT).
    mov ah, 0eh
    ; Call BIOS function.
    int 0x10
    ; Return from the subroutine.
    ret

; Create a label called message that contains the string "Hello Fun OS",
; followed by the null terminator (0) at the end.
message: db "Hello Fun OS", 0

; The following two instructions are only used to create the boot signature.
; Boot signature 0x55AA on the last two bytes of the sector.
; Fill the rest of the 510 bytes of data by padding unused bytes with zeros.
times 510 - ($ - $$) db 0
; Intel machines are little-endian, so bytes get flipped when working with words.
; (Word is assembled with "dw").
dw 0xAA55