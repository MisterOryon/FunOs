[BITS 32]
global_start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

_start:
    ; Disable interuption.
    cli

    ; Enable A20 if it not enable.
    in al, 0x92
    test al, 2
    jnz .after_A20
    or al, 2
    out 0x92, al

.after_A20:
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

    call kernel_main

    ; Do nothing more, enter an infinite loop.
    cli
.end_loop:
    hlt
    jmp .end_loop

; Fill the remaining bytes up to 512 with zeros.
times 512 - ($ - $$) db 0