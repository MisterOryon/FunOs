section .asm

; This function reads a single byte from an I/O port.
; Parameters:
;   - First argument [ebp+8]: Port number to read from.
; Returns:
;   - AL/EAX: The byte value read from the specified port.
global io_inb
io_inb:
    ; Set up stack frame by saving base pointer.
    push ebp
    mov ebp, esp

    ; Zero out EAX register to ensure no stale data affects the result.
    xor eax, eax
    ; Load port number from first argument (port) into EDX.
    mov edx, [ebp+8]
    ; Execute IN instruction to read one byte from port DX into AL.
    in al, dx

    ; Clean up stack frame and return (AL/EAX contains the return value).
    pop ebp
    ret

; This function reads a 16-bit word from an I/O port.
; Parameters:
;   - First argument [ebp+8]: Port number to read from.
; Returns:
;   - AX/EAX: The 16-bit value read from the specified port.
global io_inw
io_inw:
    ; Set up stack frame by saving base pointer.
    push ebp
    mov ebp, esp

    ; Zero out EAX register to ensure no stale data affects the result.
    xor eax, eax
    ; Load port number from first argument (port) into EDX.
    mov edx, [ebp+8]
    ; Execute IN instruction to read one word (16 bits) from port DX into AX.
    in ax, dx

    ; Clean up stack frame and return (AX/EAX contains the return value).
    pop ebp
    ret

; This function writes a single byte to an I/O port.
; Parameters:
;   - First argument [ebp+8]: Port number to write to.
;   - Second argument [ebp+12]: Byte value to write.
global io_outb
io_outb:
    ; Set up stack frame by saving base pointer.
    push ebp
    mov ebp, esp

    ; Load value to output from second argument into EAX.
    mov eax, [ebp+12]
    ; Load port number from first argument into EDX.
    mov edx, [ebp+8]
    ; Execute OUT instruction to write one byte (AL) to port DX.
    out dx, al

    ; Clean up stack frame and return.
    pop ebp
    ret

; This function writes a 16-bit word to an I/O port.
; Parameters:
;   - First argument [ebp+8]: Port number to write to.
;   - Second argument [ebp+12]: 16-bit value to write.
global io_outw
io_outw:
    ; Set up stack frame by saving base pointer.
    push ebp
    mov ebp, esp

    ; Load value to output from second argument into EAX.
    mov eax, [ebp+12]
    ; Load port number from first argument into EDX.
    mov edx, [ebp+8]
    ; Execute OUT instruction to write one word (16 bits, AX) to port DX.
    out dx, ax

    ; Clean up stack frame and return.
    pop ebp
    ret