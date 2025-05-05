# FunOS docs

The `docs` folder contains all the information about how FunOS works, except for the resources within the `learning`
folder (see [Learning docs](#learning-docs) for more details).

## x86 Architecture

The `x86` folder provides detailed information specific to the x86 architecture.

### Boot Sequence

Detailed information about how FunOS boots on x86 architecture can be found in
the [boot sequence documentation](x86/boot.md).

### Kernel Startup

The requirements and setup done by FunOS during startup can be found in the [kernel_startup](x86/kernel_startup.md) doc.

### Display

For details about how to print messages on the screen, see the [display](x86/display.md) doc.

# Learning docs

The **learning folder** contains a compilation of information that I use to build **FunOS**.
It is not a formal documentation about how computers work but rather a place where I store notes and important concepts
that I need to keep in mind for reference.

## x86 Architecture

### BIOS Formats

- **[MBR](learning/x86/bios.md#mbr-master-boot-record)**: Master Boot Record, an important structure for booting
  processes in BIOS.

### Boot System

- **[Bootloader](learning/x86/bootloader.md)**: Basics of the bootloader and its role in the startup process.

### Disk Access

- **[BIOS Call](learning/x86/bios.md#disk-access)**: How the BIOS handles reading from disks.
- **[ATA Driver](learning/x86/ata.md)**: Details about the driver for ATA (Advanced Technology Attachment) storage
  devices.

### Processor Modes

- **[Real Mode](learning/x86/real_mode.md)**: Overview of processor operations in Real Mode.
- **[Protected Mode](learning/x86/protected_mode.md)**: Overview of processor operations in Protected Mode.

### Memory

- **[Segment](learning/x86/real_mode.md#memory)**: Memory segmentation in Real Mode.
- **[GDT](learning/x86/protected_mode.md#memory)**: Global Descriptor Table used in Protected Mode.

### Interrupts

- **[IVT](learning/x86/interrupts.md#ivt)**: The Interrupt Vector Table used for handling system interrupts in real
  mode.
- **[IDT](learning/x86/interrupts.md#idt)**: The Interrupt Descriptor Table used for handling system interrupts in
  protected mode.
- **[Real Mode Exception Interrupts](learning/x86/real_mode.md)**: Exception handling in real mode.

### Protection Rings

- **[32-bit Rings](learning/x86/protected_mode.md#protection-rings-in-32-bit-protected-mode)**: The concept of rings,
  including privilege levels in 32-bit Protected Mode.

### CPU Specification

- **[Registers](learning/x86/registers.md)**: Details about CPU registers and their role in executing instructions.

### Screen

- **[Text Mode](learning/x86/text_mode.md)**: A mode where the screen is represented as a grid of characters,
  each with customizable colors.

### I/O Port

- **[IN And OUT](learning/x86/io_port.md)** : I/O port used for communication between the processor and peripheral
  devices.

### PIC (Programmable Interrupt Controller)

- **[PIC](learning/pic.md)**: Hardware component in computer systems that manages hardware interrupt requests (IRQs) and
  prioritizes them before sending to the CPU.