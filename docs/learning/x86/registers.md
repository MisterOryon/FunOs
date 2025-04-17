# Registers

## 8-bit Registers (x86)

The 8-bit registers can be used for operations on very simple integers or to manipulate parts of a larger register.

- **AH**, **AL**: High and low parts of AX
- **BH**, **BL**: High and low parts of BX
- **CH**, **CL**: High and low parts of CX
- **DH**, **DL**: High and low parts of DX

**Example**:

- AX (16 bits) can be divided into **AH** (8 high bits) and **AL** (8 low bits).

## 16-bit Registers (x86)

16-bit registers were predominant in the early x86 architectures.

- **AX**: Accumulator Register
- **BX**: Base Register
- **CX**: Counter Register
- **DX**: Data Register
- **SP**: Stack Pointer (points to the top of the stack)
- **BP**: Base Pointer (used to reference a base within the stack)
- **SI**: Source Index (mainly used for memory manipulation)
- **DI**: Destination Index (used for string operations)
- **IP**: Instruction Pointer (points to the next instruction to execute)
- **FLAGS**: Indicator (stores flags like Carry, Zero, etc.)

## 32-bit Registers (x86)

With the introduction of 32-bit architectures (e.g., in protected mode), some registers were extended to 32 bits, and
their prefix changed (adding an **E**) to identify that they were extended.

- **EAX**: Accumulator Register
- **EBX**: Base Register
- **ECX**: Counter Register
- **EDX**: Data Register
- **ESI**: Source Index
- **EDI**: Destination Index
- **EBP**: Base Pointer
- **ESP**: Stack Pointer
- **EIP**: Instruction Pointer
- **EFLAGS**: Extended flags for 32-bit processor indicators.

## 64-bit Extension (x86-64)

In modern x86_64 processors (64 bits), all registers are extended to 64 bits, and their prefix becomes **R**, for
example:

- **RAX**, **RBX**, **RCX**, **RDX**, and so on.
- Additional registers: **R8** to **R15**.

These registers also offer new capabilities and specific instructions, such as those handling floating-point numbers (
SSE/AVX).
