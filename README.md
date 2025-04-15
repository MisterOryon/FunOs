# FunOs

## Requirements

- `nasm`
- `qemu`
- `gdb`
- `make`

### On Arch Linux

Run the following command to install the required tools:
`pacman -Syu nasm qemu-full gdb make`

## Build

To build FunOs for x86 run `make`.

### Bootloader

The size of the bootloader is always 512 bytes.
To view the disassembly output, you can run: `ndisasm ./bin/x86_boot_loader.bin`.

## Run

To run FunOS, use the following command: `qemu-system-x86_64 -hda ./bin/x86_boot_loader.bin`

## Run in Debugging Environment

To run FunOS in a debugging environment, use the following command:

```bash
qemu-system-x86_64 -s -S -hda ./bin/x86_boot_loader.bin
```

- **`-s`**: Shorthand for `-gdb tcp::1234`, which starts a GDB server listening on TCP port 1234.
- **`-S`**: Makes QEMU stop execution at the very beginning until you tell it to continue using GDB.

This allows you to connect a debugger like GDB to debug FunOS step-by-step.

### GDB

To connect to QEMU with GDB, issue the following command inside GDB:

```bash
target remote localhost:1234
```

#### Setting Breakpoints

Place your breakpoint with the command:

```bash
break *<addr>
```

In FunOS, the following address is interesting:

- **0x7c00**: This is the memory address where the bootloader is loaded.

#### Debugging in GDB

1. After placing your breakpoint, use the `continue` command to start execution.
2. Use the `layout asm` command to view the assembly layout.
3. To step through the code instruction by instruction, use the `stepi` command to move to the next instruction.

This approach allows you to debug and understand the step-by-step execution of FunOS.

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

### Protected Mode

Protected mode is a more advanced mode compared to real mode.
It provides several features such as access to more memory (up to 4GB) and 32-bit support.  
Protected mode also allows the following features:

- **Memory Protection**: Each program has its own portion of memory, and the processor ensures that no program can
  interfere with the memory of another.
- **Virtual Memory**: Allows multiple programs to share the same virtual address space.
  It also permits the extension of main memory by using disk space.
- **Multitasking**: The processor can switch between different tasks, giving each task a slice of processor time.

Switching to protected mode is a crucial step in the boot process as it enables these advanced features.

#### Entering Protected Mode

The general steps involved in the process are:

1. **Disable Interrupts**: This is done because the Interrupt Vector Table (IVT) location and format change when moving
   to protected mode.
   If an interrupt occurs during the setup, the system could crash.
2. **Set up a Global Descriptor Table (GDT)**: The GDT describes the memory segments, including the base address, size
   limit, and access rights for each segment.
3. **Load the GDT**: The GDT address must be loaded into the GDTR (Global Descriptor Table Register) using the `lgdt`
   instruction.
4. **Set the Protected Mode Bit**: In the CR0 (Control Register 0), the first bit (bit 0) must be set to 1 to enable
   protected mode.
5. **Perform a Far Jump**: A far jump is necessary to clear the prefetch queue and ensure that the processor starts
   executing instructions in protected mode.
   The CS (Code Segment) register is loaded with a selector from the GDT.
6. **Set up Segment Registers**: The remaining segment registers (DS, SS, ES, FS, GS) are loaded with selectors from the
   GDT.
   The stack pointer (SP or ESP) is also set up at this time.
7. **Enable Interrupts (if necessary)**: Once all the above steps are completed, interrupts can be re-enabled.

#### Example Assembly Code to Enter Protected Mode:

```asm
cli                           ; Disable interrupts
lgdt [gdt_descriptor]         ; Load the GDT into GDTR
mov eax, cr0                  ; Load CR0 into EAX
or eax, 1                     ; Set the protected mode bit (bit 0) in CR0
mov cr0, eax                  ; Write CR0 to enable protected mode
jmp CODE_SEG:next_instruction ; Perform a far jump to clear the prefetch queue

next_instruction:
mov ax, DATA_SEG           ; Load data segment selector from GDT
mov ds, ax                 ; Set data segment register
mov es, ax                 ; Set extra segment register
mov ss, ax                 ; Set stack segment register
mov esp, 0x200000          ; Set up the stack pointer
sti                        ; Enable interrupts
```

#### Enabling the A20 Line

The A20 line controls whether the CPU can address memory beyond the 1 MB boundary.
Disabling it restricts memory addressing to 20 bits (real mode), while enabling it allows access to all memory supported
by the CPU.

To access all available memory (4 GB) beyond the 1 MB limit of real mode, we need to enable the A20 line.
The following assembly code reads the content of port `0x92` (System Control Port A),
enables the second bit (responsible for controlling the A20 line), and writes the new contents back to port `0x92`:

```asm
in al, 0x92         ; Read the value from port 0x92 into the AL register
or al, 2            ; Enable the second bit (A20 line)
out 0x92, al        ; Write the updated value back to port 0x92
```

## Memory

### Segment Registers

In real mode or with the Intel 8086 processor, you have the following segment registers:

- **CS (Code Segment)**
- **SS (Stack Segment)**
- **DS (Data Segment)**
- **ES (Extra Segment)**

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

### Global Descriptor Table (GDT)

When protected mode is enabled, the processor uses the Global Descriptor Table (GDT) to define the characteristics of
the segments it will use.
Each entry in the GDT is a segment descriptor that provides information about a segment, such as its size, location, and
access rights.
The first entry in the GDT (Entry 0) should always be null and later entries should be used instead.

#### GDT Descriptors

Each descriptor in the GDT is 8 bytes long and follows this format:

| 0:15 Lim. L | 16:31 B. L | 32:39 B. M | 40:47 Acc. B | 48:51 Lim. H | 52:55 Flg | 56:63 B. H |
|-------------|------------|------------|--------------|--------------|-----------|------------|
| `0x0000`    | `0x0000`   | `0x00`     | `0x00`       | `0x0`        | `0x0`     | `0x00`     |

##### Descriptor Fields Explained

1. **Segment Limit**:
    - This 20-bit field is split between bits 0–15 (`Lim. L`) and bits 48–51 (`Lim. H`).
    - It specifies the size of the segment. The actual size is computed as:
      ``` 
      (Segment Limit + 1) * Granularity
      ```
2. **Base Address**:
    - This is a 32-bit field that specifies the linear address where the segment begins.
    - It is split between `B. L` (bits 16–31), `B. M` (bits 32–39), and `B. H` (bits 56–63).
3. **Access Byte**:  
   This byte specifies the type of segment, its privilege level, and other characteristics. It is divided into several
   bitfields:
    - **Bit 0**: Accessed bit.
    - **Bit 1**: Readable bit for a code segment or writable bit for a data segment.
    - **Bit 2**: Direction bit for data selectors or conforming bit for code selectors.
    - **Bit 3**: Executable bit. If set, the segment is a code segment.
    - **Bit 4**: Descriptor bit. Should be set to 1 for code or data segments.
    - **Bits 6-5**: Descriptor Privilege Level (DPL).
      Specify the privilege level of the segment, ranging from 0 (highest privilege) to 3 (lowest privilege).
    - **Bit 7**: Segment Present (P) bit. Should be set to 1 for valid segments.
4. **Flags**:
    - These include the Granularity (G) bit (bit 3) and the Size (S) bit (bit 2).
    - The **Granularity bit (G, bit 3)**:
        - When set to 1, the segment limit is specified in 4 KB blocks instead of bytes.
        - When set to 0, the limit is in bytes.
    - The **Size bit (S, bit 2)**:
        - Specifies whether the segment uses 32-bit protected mode (1) or 16-bit modes (0).

##### Example: Writing a Descriptor in Assembly

```asm
dw 0xffff          ; Segment Limit (Low 16 bits)
dw 0x0000          ; Base Address (Low 16 bits)
db 0x00            ; Base Address (Middle 8 bits)
db 0x9a            ; Access Byte (code segment, present, privilege 0, executable)
db 11001111b       ; Flags (Granularity: 1, 32-bit size: 1) + Segment Limit (High 4 bits)
db 0x00            ; Base Address (High 8 bits)
```

#### Using the GDT

To utilize the GDT, we need to set the GDTR register with the `lgdt` instruction.
When the processor shifts into protected mode, each of these segment registers (DS, ES, ...) is transformed into a
segment selector that points to an index within the GDT.
After that, the absolute address is calculated by following the GDT parameters.

Note that the GDT descriptor passed to the `lgdt` instruction must specify the size of the GDT and the starting point of
the GDT.

## Interrupts

In computers, an interrupt is a signal sent to the processor when an event occurs and requires immediate attention.
These allow hardware devices and software to communicate with the CPU by interrupting its current activities.

There are 3 kinds of interrupts:

- **Hardware interrupts**: Devices like keyboards, mouse, timers, etc., can trigger interrupts because they need the
  processor to process their input (e.g., a key press on a keyboard).
- **Software interrupts**: Software or applications can trigger interrupts to request a service from the OS, such as
  reading a file, sending data over a network, etc.
- **Exceptions**: These interrupts occur when an error happens, such as division by zero.

When an interrupt is triggered, the processor uses the IVT (Interrupt Vector Table) to determine how to handle it.
Inside the IVT, the processor can locate the appropriate ISR (Interrupt Service Routine) to start a small piece of
code that handles the specific interrupt.

When an interrupt is triggered, the processor saves its current state and jumps to the ISR specified in the IVT.
Once the ISR is completed by executing the IRET (Return from Interrupt) instruction, the processor state is restored,
and execution resumes.

Note that interrupt number `0x80` is commonly used by software interrupts in kernel development, and it is not used by
any BIOS routines.

### IVT

The IVT (Interrupt Vector Table) is a table residing in memory at address `0x0000` in real mode.
It contains 256 entries, each 4 bytes long (2 bytes for the offset followed by 2 bytes for the segment).
The total size is 1 KB (1024 bytes), and each entry points to an ISR (a piece of code).

The equation to calculate the offset is: `interrupt_num * size_of_entry = offset to edit`

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

## Disk Access

The BIOS interrupt `int 0x13` provides an interface in real mode to read and write to an HDD (hard disk drive).  
At a low level, the disk is divided into sectors, which are typically 512 bytes in size, though some modern disks use
larger sectors.  
For an HDD, sectors are physically arranged in concentric circles called tracks.
The disk is divided into smaller units called cylinders, heads, and sectors, which are used to address individual
sectors.

To read from the disk, CPU registers need to be set as follows:

- **AH register**: The function number (`0x02` to read from the disk).
- **AL register**: The number of sectors to read.
- **CH register**: The cylinder number.
- **CL register**: The sector number.
- **DH register**: The head number.
- **DL register**: The drive number (the first hard drive is typically `0x80`).
- **ES:BX register**: The address of the buffer to store the data read from the disk.

Writing to the disk is similar, but **AH** is set to `0x03`, and **ES:BX** points to the data to write.

## MBR (Master Boot Record)

The MBR (Master Boot Record) is a special type of boot sector that resides at the beginning of a partitioned computer
disk.  
The structure required by the MBR format is as follows:

- **Boot Code (446 bytes)**: This is the code that the BIOS reads and executes as the first step in the boot process.
- **Partition Table (64 bytes)**: This section contains the disk's partition table. The partition table is an array of
  four entries, each being 16 bytes long, and each describing a partition on the disk.
- **Boot Signature (2 bytes)**: The last two bytes always hold `0x55AA` and are used by the BIOS to verify the integrity
  of the MBR sector.

### Sample Minimal MBR Assembly Code:

```asm
org 0x7c00            ; Origin: Marks the executable location (loaded at 0x7c00)
jmp short start       ; Jump to the main boot code
nop                   ; No operation (padding)

start:
    ; Bootloader code would go here

times 446-($-$$) db 0 ; Fill the remainder of the boot sector with zeros
times 64 db 0         ; Fill partition table of the boot sector with zeros
dw 0xAA55             ; Boot signature (little-endian, the bytes are flipped)
```

Explanation:

- The `times` directive is used to fill up the remaining space of the boot sector (510 bytes total) with zeros.
  This ensures the total size of the sector (excluding the boot signature) is exactly 510 bytes.
- The `dw 0xAA55` is the boot signature, stored in little-endian format, which means the bytes will be written as `0x55`
  followed by `0xAA`.
  This is located at the last two bytes of the 512-byte MBR sector, as required by the BIOS.

## Protection Rings in 32-bit Protected Mode

Four privilege levels are present in 32-bit protected mode, known as protection rings.
These levels are utilized as follows:

- **Ring 0**:  
  This is the kernel level and the most privileged ring.
  It allows direct access to physical hardware, such as memory and I/O operations.

- **Ring 1 and Ring 2**:  
  These are intermediate levels that typically contain device drivers and other critical services.
  They need more privileges than user applications but should not have full access to the kernel.

- **Ring 3**:  
  This is the least privileged level, where user-mode applications reside.
  Applications request resources and services through system calls to interact with the kernel.

In practice, most modern operating systems only use **Ring 0** (kernel mode) and **Ring 3** (user mode).

## ATA Drives

ATA (Advanced Technology Attachment) is a standard interface for connecting storage such as hard drives and solid-state
drives to the motherboard.  
In OS kernels, ATA is also commonly referred to as IDE (Integrated Drive Electronics).  
ATA drives play an essential role by allowing the BIOS or UEFI to read the bootloader from the disk and enabling the
kernel to communicate with the disk.

### Communicating with ATA Drives

To communicate with an ATA drive, we use commands that are sent and responses that are read through the drive's control
registers.  
These registers are mapped to certain addresses in I/O space and accessed in assembly using the `in` and `out`
instructions.

In most cases, an operating system will abstract away these details by providing higher-level APIs for accessing storage
devices.

### LBA vs CHS

When working with storage, there are two ways to specify the location of data on the disk: LBA (Logical Block
Addressing) and CHS (Cylinder-Head-Sector).

Note that BIOS using interrupt `0x13` provides a method to use the CHS format.

#### CHS

CHS addressing is an older method of addressing disks that involves specifying the Cylinder, Head, and Sector where data
is located.

- **Cylinder**: A vertical slice through the platters of a hard drive consisting of all the tracks that are at the same
  distance from the center of the platters.
- **Head**: Refers to the read/write head that accesses the platter surface where sectors are stored.
- **Sector**: A subdivision of a track on the disk (typically 512 bytes).

The main advantage of CHS is that it directly corresponds to the physical structure of a hard disk.
However, it is complex to work with and does not scale well with larger drives.

#### LBA

LBA provides an abstract method to perform access.
Instead of using cylinders, heads, and sectors, data is accessed by specifying a simple block number starting from 0.

The main disadvantage of LBA is that it abstracts the physical layout of the disk, which can potentially lead to less
efficient data access patterns.  
Today all modern operating systems and BIOS/UEFI implementations use LBA for disk access.
