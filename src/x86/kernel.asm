[BITS 32]
global_start

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    ; Enable the A20 line
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
    mov ebp, 0x00200000
    mov esp, ebp

    jmp $

; Fill remaining bytes up to 512 with 0.
times 512 - ($ - $$) db 0