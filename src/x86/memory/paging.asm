[BITS 32]

section .asm

global paging_load_directory
global enable_paging

; This function modifies the paging layout.
; Parameters:
;   - [ebp+8]: Physical address of the Page Directory to be loaded into CR3.
;              Must be 4KB-aligned (12 least significant bits must be zero).
paging_load_directory:
    ; Save the current base pointer (EBP) to set up a stack frame for the function.
    push ebp
    mov ebp, esp

    ; Switch paging layout by loading the physical address of the new Page Directory into CR3.
    mov eax, [ebp+8]
    mov cr3, eax

    ; Restore the previous base pointer and clean up the stack frame.
    pop ebp
    ret

; Enables memory paging by setting the PG bit (bit 31) in CR0 control register.
; Note: Before enabling paging, a valid page directory must be loaded into CR3,
; and the PE bit (bit 0) in CR0 must already be set to enable protected mode.
enable_paging:
    ; Save the current base pointer (EBP) to set up a stack frame for the function.
    push ebp
    mov ebp, esp

    ; Enable paging by reading CR0, setting the PG bit (bit 31) and PW (bit 16), then writing back to CR0.
    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax

    ; Restore the previous base pointer and clean up the stack frame.
    pop ebp
    ret
