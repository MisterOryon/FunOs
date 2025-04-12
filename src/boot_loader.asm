; Tell the assembler that the origin address (offset) is 0x7c00,
; because the BIOS loads bootloader code at this part of memory.
ORG 0x7c00

; Tell the assembler that we are using 16-bit architecture.
BITS 16

start:
    ; Ask the BIOS to display a character on the screen.
    ; INT 10/AH=0Eh (VIDEO - TELETYPE OUTPUT)
    mov ah, 0eh
    ; Character to write on the screen.
    mov al, 'A'
    ; Call BIOS function.
    int 0x10

    ; Jump to itself to ensure we don't execute unwanted instructions beyond the signature.
    ; The following two instructions are only used to create the boot signature.
    jmp $

; Boot signature 0x55AA on the last two bytes of the sector.
; Fill the rest of the 510 bytes of data by padding unused bytes with zeros.
times 510 - ($ - $$) db 0
; Intel machines are little-endian, so bytes get flipped when working with words.
; (Word is assembled with "dw").
dw 0xAA55