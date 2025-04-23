section .asm

; This function loads the address of an Interrupt Descriptor Table (IDT) into the CPU using the `lidt` instruction.
; The first argument to the function should be a pointer to the IDT structure.
global idt_load
idt_load:
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