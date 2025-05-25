section .asm

extern handle_keyboard_interrupt
extern handler_no_interrupt

; This function loads the address of an Interrupt Descriptor Table (IDT) into the CPU using the `lidt` instruction.
; The first argument to the function should be a pointer to the IDT structure.
global idt_load_table
idt_load_table:
    ; Save the current base pointer (EBP) to set up a stack frame for the function.
    push ebp
    mov ebp, esp

    ; Retrieve the first argument (the pointer to the IDT) from the stack.
    mov ebx, [ebp+8]
    ; Load the IDT into the CPU.
    lidt [ebx]

    ; Restore the previous base pointer and clean up the stack frame.
    pop ebp
    ret

; This function serves as the entry point for keyboard interrupts and calls the C implementation
; handle_keyboard_interrupt().
global idt_handle_keyboard_interrupt
idt_handle_keyboard_interrupt:
    ; Disable interrupts.
    cli
    ; Push all general-purpose registers.
    ; The pushad instruction quickly saves the entire CPU register state so that the interrupt handler doesn't corrupt
    ; the interrupted program's execution context.
    pushad
    ; Call the C/C++ handler function.
    call handle_keyboard_interrupt
    ; Restore all general-purpose registers.
    popad
    ; Enable interrupts.
    sti
    ; Return from interrupt
    iret

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
