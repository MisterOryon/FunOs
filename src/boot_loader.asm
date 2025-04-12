; Tell the assembler that the origin address (offset) is 0x7c00,
; because the BIOS loads bootloader code at this part of memory.
ORG 0x7c00

; Tell the assembler that we are using 16-bit architecture.
BITS 16

start:
    ; Move the address of the message label into the SI register.
    mov si, message
    call print

    ; Jump to itself to ensure we don't execute unwanted instructions beyond the signature.
    jmp $

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
    ; Return from the subroutine.
    ret

print_char:
    ; Ask the BIOS to display a character on the screen.
    ; INT 10/AH=0Eh (VIDEO - TELETYPE OUTPUT)
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