# Real Mode

Real mode is the compatibility mode that all modern Intel processors start in when they are powered on.  
Real mode mimics the behavior of processors from many years ago (Intel 8086) and is based on the original x86 design
from the 1970s.  
The first x86 processors did not provide any security features, such as memory protection or privilege levels (rings).

In real mode, we can only access 8- and 16-bit registers.
Additionally, in real mode, only 16-bit code is allowed, which means we can only work with numbers between 0 and
65,536.  
Memory is accessed using a segmentation model, which means memory can be accessed using segments and offsets.  
The reason we can address up to one megabyte of memory, despite the 65,536 limit, is that segment registers are taken
into account.

Modern code running in real mode faces the same limitations as the old processors (e.g., 16-bit constraints, memory
models, x86 assembly code, etc.).

## Memory

### Segment Registers

In real mode or with the Intel 8086 processor, you have the following segment registers:

- **CS (Code Segment)**
- **SS (Stack Segment)**
- **DS (Data Segment)**
- **ES (Extra Segment)**

In protected mode, additional segment registers `FS` and `GS` are available.
The stack pointer (SP) is extended to **ESP** (Extended Stack Pointer) in 32-bit mode,
providing access to a larger address space.

#### Calculating Absolute Offset

The absolute position in RAM can be calculated by taking the segment register, multiplying it by 16, and adding the
offset.  
This combination allows addressing up to one megabyte of memory.

Note that the offset is defined by the `ORG` (Assembly Origin) directive in assembly code.

#### Instructions Use Different Segment Registers

The `lodsb` instruction uses the `DS:SI` register combination—more precisely, it uses `DS` as the segment register and
`SI` as the offset register.

Example in Assembly:

```asm
ORG 0
; Move 0x7C0 into the AX register.
mov ax, 0x7C0
; Move AX into DS to set the data segment to 0x7C0.
mov ds, ax
; Move 0x1F into the SI register (points to a message).
mov si, 0x1F
; LODSB now calculates the address to read using the following:
; 0x7C0 * 16 = 0x7C00
; 0x7C00 + 0x1F = 0x7C1F
; It needs to read the byte at address 0x7C1F.
lodsb
```

#### Programs Can Be Loaded in Different Areas of Memory

You can swap the segment register when switching to another process, and by restoring the register values of a process,
the program resumes as if it had never switched.

For example:

- **Program 1** is loaded at address 0x7C00 and uses segment 0x7C0 for all its segment registers.
- **Program 2** is loaded at address 0x7D00 and uses segment 0x7D0 for all its segment registers.

#### Multiple Segments Are Available Using Segment Registers

Examples of reading memory with different segment registers:

```asm
; Read memory at physical address (ES segment register * 16) + 32 and store it into AL.
mov al, [es:32]
; Read memory at physical address (DE segment register * 16) + 826 and store it into AL.
mov al, [de:826]
; Read memory at physical address (SS segment register * 16) + 231 and store it into AL.
mov al, [ss:231]
```

#### Stack Segment

The stack pointer (SP) points to a location in memory, and stack operations are based on the stack segment (SS) and the
stack pointer.  
More precisely, the absolute address is calculated by multiplying the stack segment by 16 and adding the stack pointer.

For example:

- We start with `SP` (Stack Pointer) at 0x7C00 and `SS` (Stack Segment) at 0x00.
- When using `push 0xffff`, we decrement the stack pointer by 2 because we push two bytes onto the stack.
- Now, the stack pointer becomes 0x7BFE, and 0xFFFF is stored in memory at addresses 0x7BFE and 0x7BFF.

## Interrupts

### Real Mode Exception Interrupts

| Interrupt | Description                      |
|-----------|----------------------------------|
| INT 0x00  | Divide by zero                   |
| INT 0x01  | Single step (Debug)              |
| INT 0x02  | Non-maskable interrupt           |
| INT 0x03  | Breakpoint (Debug)               |
| INT 0x04  | Overflow (INTO instruction)      |
| INT 0x05  | Bounds check (BOUND instruction) |
| INT 0x06  | Invalid Opcode                   |
| INT 0x07  | Coprocessor not available        |
| INT 0x08  | Double Fault                     |
| INT 0x09  | Coprocessor Segment Overrun      |
| INT 0x0A  | Invalid Task State Segment       |
| INT 0x0B  | Segment not present              |
| INT 0x0C  | Stack-segment fault              |
| INT 0x0D  | General Protection Fault         |
| INT 0x0F  | Reserved (Intel use only)        |
| INT 0x10  | Coprocessor Error                |
