# Protected Mode

Protected mode is a more advanced mode compared to real mode.
It provides several features such as access to more memory (up to 4GB) and 32-bit support.  
Protected mode also allows the following features:

- **Memory Protection**: Each program has its own portion of memory, and the processor ensures that no program can
  interfere with the memory of another.
- **Virtual Memory**: Allows multiple programs to share the same virtual address space.
  It also permits the extension of main memory by using disk space.
- **Multitasking**: The processor can switch between different tasks, giving each task a slice of processor time.

Switching to protected mode is a crucial step in the boot process as it enables these advanced features.

## Entering Protected Mode

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

### Example Assembly Code to Enter Protected Mode:

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

### Enabling the A20 Line

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

### Global Descriptor Table (GDT)

When protected mode is enabled, the processor uses the Global Descriptor Table (GDT) to define the characteristics of
the segments it will use.
Each entry in the GDT is a segment descriptor that provides information about a segment, such as its size, location, and
access rights.
The first entry in the GDT (Entry 0) should always be null and later entries should be used instead.

##### GDT Descriptors

Each descriptor in the GDT is 8 bytes long and follows this format:

| 0:15 Lim. L | 16:31 B. L | 32:39 B. M | 40:47 Acc. B | 48:51 Lim. H | 52:55 Flg | 56:63 B. H |
|-------------|------------|------------|--------------|--------------|-----------|------------|
| `0x0000`    | `0x0000`   | `0x00`     | `0x00`       | `0x0`        | `0x0`     | `0x00`     |

###### Descriptor Fields Explained

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

###### Example: Writing a Descriptor in Assembly

```asm
dw 0xffff          ; Segment Limit (Low 16 bits)
dw 0x0000          ; Base Address (Low 16 bits)
db 0x00            ; Base Address (Middle 8 bits)
db 0x9a            ; Access Byte (code segment, present, privilege 0, executable)
db 11001111b       ; Flags (Granularity: 1, 32-bit size: 1) + Segment Limit (High 4 bits)
db 0x00            ; Base Address (High 8 bits)
```

##### Using the GDT

To utilize the GDT, we need to set the GDTR register with the `lgdt` instruction.
When the processor shifts into protected mode, each of these segment registers (DS, ES, ...) is transformed into a
segment selector that points to an index within the GDT.
After that, the absolute address is calculated by following the GDT parameters.

Note that the GDT descriptor passed to the `lgdt` instruction must specify the size of the GDT and the starting point of
the GDT.

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
