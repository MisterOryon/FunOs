# Memory Mapping Documentation

This document describes the memory layout of the FunOs operating system, detailing the specific memory regions and their
purposes within the system architecture.

## Memory Map

| Region               | Start Address             | Size                       | Description                                          |
|----------------------|---------------------------|----------------------------|------------------------------------------------------|
| Boot Loader          | `0x00007C00`              | 512 bytes                  | Location where the BIOS loads the boot sector        |
| GDT                  | Within  boot loader space | -                          | Global Descriptor Table (memory segmentation)        |
| VGA Buffer           | `0x000B8000`              | 4,000 bytes                | Text-mode display buffer (25 rows × 80 columns)      |
| Kernel Heap Metadata | `0x00007E00`              | 25,600 bytes               | Metadata for kernel heap management                  |
| Kernel               | `0x00100000`              | 51,200 bytes               | Main kernel code                                     |
| IDT                  | Within kernel space       | 4,096 bytes                | Interrupt Descriptor Table (located in kernel space) |
| Stack                | `0x00200000`              | -                          | System stack area                                    |
| Kernel Heap          | `0x01000000`              | 104,857,600 bytes (100 MB) | Dynamic memory allocation for kernel operations      |

## Region Details

### Boot Loader (`0x7C00`)

The boot loader is loaded by the BIOS at this specific address during the system boot process.
It occupies 512 bytes and is responsible for initializing the system and loading the kernel.

### GDT (Global Descriptor Table)

The GDT is used for memory segmentation, defining memory segments and their access privileges.

### VGA Buffer (`0xB8000`)

This memory region is mapped to the video display hardware in text mode.
It supports a 25×80 character display, requiring 4,000 bytes of memory.

### Kernel Heap Metadata (`0x00007E00`)

This region stores metadata for managing the kernel heap, occupying 25,600 bytes.

### Kernel (`0x00100000`)

The main kernel code is loaded at this address, occupying 51,200 bytes.

### IDT (Interrupt Descriptor Table)

Located in kernel space, the IDT occupies 4,096 bytes and stores interrupt vectors used by the system to handle hardware
and software interrupts.

### Stack (`0x00200000`)

The system stack is located at this address and grows downward.

### Kernel Heap (`0x01000000`)

A 100 MB region allocated for dynamic memory operations by the kernel.

## Memory Layout Visualization

```text
                   Physical Memory Map
+--------------------------------------------------+ 0xFFFFFFFF
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x08594000
|                                                  |
|                   Kernel Heap                    |
|             (Dynamic Memory Region)              |
|                    (100 MB)                      |
|                                                  |
+--------------------------------------------------+ 0x01000000
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x00200000
|                                                  |
|                      Stack                       |
|        (Grows downward from this address)        |
|                                                  |
+--------------------------------------------------+ 0x00200000 - Stack Size
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x0010c800
|                     Kernel                       |
|                  (51200 bytes)                   |
|                                                  |
|         +------------------------------+         |
|         |   IDT (4096 bytes) located   |         |
|         |     within kernel space      |         |
|         +------------------------------+         |
|                                                  |
+--------------------------------------------------+ 0x00100000
|                                                  |
|             Upper Memory Area (UMA)              |
|      (Reserved for BIOS, ROM, Device Memory)     |
|                                                  |
+--------------------------------------------------+ 0x000C0000
|                                                  |
|                Video Memory Area                 |
|                                                  |
+--------------------------------------------------+ 0x000B8FA0
|                   VGA Buffer                     |
|                  (4000 bytes)                    |
|             (25 rows x 80 columns)               |
+--------------------------------------------------+ 0x000B8000
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x0000E200
|               Kernel Heap Metadata               |
|                  (25600 bytes)                   |
+--------------------------------------------------+ 0x00007E00
|                   Boot Loader                    |
|                   (512 bytes)                    |
+--------------------------------------------------+ 0x00007C00
|                                                  |
|           BIOS Data Area (BDA) & Stack           |
|                   (0x400-0x7BFF)                 |
|                                                  |
+--------------------------------------------------+ 0x00000500
|                                                  |
|        Interrupt Vector Table (IVT) for BIOS     |
|                   (0x000-0x3FF)                  |
|                                                  |
+--------------------------------------------------+ 0x00000000
```