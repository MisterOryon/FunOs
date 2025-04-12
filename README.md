# FunOs

## Requirements

- `nasm`
- `qemu`

### On Arch Linux

Run the following command to install the required tools:
`pacman -Syu nasm qemu-full`

## Build

### Bootloader

To build the bootloader, run the following command: `nasm -f bin src/boot_loader.asm -o dist/boot_loader.bin`.
The size of the bootloader is always 512 bytes.
To view the disassembly output, you can run: `ndisasm ./dist/boot_loader.bin`.

## Run

To run FunOS, use the following command: `qemu-system-x86_64 -hda ./dist/boot_loader.bin`

## Booted System

### BIOS

The BIOS is generally loaded into RAM and then continues execution from there.
Its job is to initialize hardware and provide basic functionality, such as drivers for peripherals (e.g., disk drivers).

Once hardware initialization is complete, the BIOS will attempt to load a bootloader by searching all storage mediums
for a boot signature (`0x55AA`).  
If the BIOS finds this signature in the first sector of a storage medium, it will load the sector into memory at address
`0x7C00` and execute the bootloader from that address.

A sector is a block of storage; for hard disks, one sector is 512 bytes. Specifically, the BIOS checks the first sector
of each storage medium to see if bytes 511 and 512 contain the boot signature.

The BIOS runs 16-bit code, which means its functions are usable only when the processor is in "real mode."

You can find a list of all available BIOS functions
at: [https://www.ctyme.com/rbrown.htm](https://www.ctyme.com/rbrown.htm)

### Bootloader

A bootloader is a small program responsible for loading the kernel. Bootloaders are generally small in size.

When a computer boots, it starts in a compatibility mode called "real mode."  
This mode provides access to up to one megabyte of memory and runs 16-bit code.

The responsibility of our bootloader is to transition the processor into "protected mode," which provides access to four
gigabytes of memory and runs 32-bit code.  
After entering protected mode, the job of the bootloader is to load the kernel into memory and transfer execution to the
kernel.

Bootloaders generally use the functions provided by the BIOS to assist in their tasks.

## Processor mode

### Real Mode

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

#### Memory

##### Segment Registers

In real mode or with the Intel 8086 processor, you have the following segment registers:

- **CS (Code Segment)**
- **SS (Stack Segment)**
- **DS (Data Segment)**
- **ES (Extra Segment)**

##### Calculating Absolute Offset

The absolute position in RAM can be calculated by taking the segment register, multiplying it by 16, and adding the
offset.  
This combination allows addressing up to one megabyte of memory.

Note that the offset is defined by the `ORG` (Assembly Origin) directive in assembly code.

##### Instructions Use Different Segment Registers

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

##### Programs Can Be Loaded in Different Areas of Memory

You can swap the segment register when switching to another process, and by restoring the register values of a process,
the program resumes as if it had never switched.

For example:

- **Program 1** is loaded at address 0x7C00 and uses segment 0x7C0 for all its segment registers.
- **Program 2** is loaded at address 0x7D00 and uses segment 0x7D0 for all its segment registers.

##### Multiple Segments Are Available Using Segment Registers

Examples of reading memory with different segment registers:

```asm
; Read memory at physical address (ES segment register * 16) + 32 and store it into AL.
mov al, [es:32]
; Read memory at physical address (DE segment register * 16) + 826 and store it into AL.
mov al, [de:826]
; Read memory at physical address (SS segment register * 16) + 231 and store it into AL.
mov al, [ss:231]
```

##### Stack Segment

The stack pointer (SP) points to a location in memory, and stack operations are based on the stack segment (SS) and the
stack pointer.  
More precisely, the absolute address is calculated by multiplying the stack segment by 16 and adding the stack pointer.

For example:

- We start with `SP` (Stack Pointer) at 0x7C00 and `SS` (Stack Segment) at 0x00.
- When using `push 0xffff`, we decrement the stack pointer by 2 because we push two bytes onto the stack.
- Now, the stack pointer becomes 0x7BFE, and 0xFFFF is stored in memory at addresses 0x7BFE and 0x7BFF.
