section .asm

extern handler_no_interrupt
extern handle_kernel_panic
extern handle_keyboard_interrupt

; This function is used as the default handler for interrupts without specific handlers and calls the C
; implementation handler_no_interrupt().
global idt_no_interrupt
idt_no_interrupt:
     ; Disable interrupts.
    cli
    ; Push all general-purpose registers.
    ; The pushad instruction quickly saves the entire CPU register state so that the interrupt handler doesn't corrupt
    ; the interrupted program's execution context.
    pushad
    ; Call the C/C++ handler function.
    call handler_no_interrupt
    ; Restore all general-purpose registers.
    popad
    ; Enable interrupts.
    sti
    ; Return from interrupt
    iret

; This function serves as the entry point for keyboard interrupts and calls the C implementation
; handle_keyboard_interrupt().
global idt_handle_keyboard_interrupt
idt_handle_keyboard_interrupt:
    cli
    pushad
    call handle_keyboard_interrupt
    popad
    sti
    iret

; This function serves as the entry point for x86 exception and calls C implementation handle_kernel_panic().
global idt_handle_kernel_panic
idt_handle_kernel_panic:
    cli
    pushad
    call handle_kernel_panic
    popad
    sti
    iret

