# Memory Mapping Documentation

This document describes the memory layout of the FunOs operating system, detailing the specific memory regions and their
purposes within the system architecture.

## Memory Map

| Region               | Start Address             | Size                       | Description                                          |
|----------------------|---------------------------|----------------------------|------------------------------------------------------|
| Boot Loader          | `0x00007C00`              | 512 bytes                  | Location where the BIOS loads the boot sector        |
| GDT                  | Within  boot loader space | -                          | Global Descriptor Table (memory segmentation)        |
| VGA Buffer           | `0x000B8000`              | 4,000 bytes                | Text-mode display buffer (25 rows × 80 columns)      |
| Kernel Heap Metadata | `0x00008000`              | 25,600 bytes               | Metadata for kernel heap management                  |
| Kernel KFLASH        | `0x00100000`              | 32,768 bytes               | Main kernel code                                     |
| Kernel DRAM          | `0x00108000`              | 65,536 bytes               | Main kernel data                                     |
| IDT                  | Within kernel DRAM space  | 4,096 bytes                | Interrupt Descriptor Table (located in kernel space) |
| Stack Guard          | `0x00118000`              | 8,192 bytes                | System stack area                                    |
| Stack                | `0x00400000`              | -                          | System stack area                                    |
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

### Kernel Heap Metadata (`0x00008000`)

This region stores metadata for managing the kernel heap, occupying 25,600 bytes.

### Kernel KFLASH (`0x00100000`)

The main kernel code is loaded at this address, occupying 32,768 bytes.

### Kernel DRAM (`0x00108000`)

The main kernel data is loaded at this address, occupying 65,536 bytes.

### IDT (Interrupt Descriptor Table)

Located in the kernel DRAM space, the IDT occupies 4,096 bytes and stores interrupt vectors used by the system to handle
hardware and software interrupts.

### Stack Guard (`0x00118000`)

FunOs using stack guard of 8,192 bytes to ensure that there will be no stack overfull that will overwrite the kernel.

### Stack (`0x00400000`)

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
+--------------------------------------------------+ 0x07400000
|                                                  |
|                   Kernel Heap                    |
|             (Dynamic Memory Region)              |
|                    (100 MB)                      |
|                                                  |
+--------------------------------------------------+ 0x01000000
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x00400000
|                                                  |
|                      Stack                       |
|        (Grows downward from this address)        |
|                                                  |
+--------------------------------------------------+ 0x00400000 - Stack Size
|                                                  |
|                  (Unused Space)                  |
|                                                  |
+--------------------------------------------------+ 0x0011A000
|                  (Stack Guard)                   |
|                   (8192 bytes)                   |
+--------------------------------------------------+ 0x00118000
|                                                  |
|            Kernel DRAM (.data, .bss)             |
|                  (65,536 bytes)                  |
|                                                  |
|         +------------------------------+         |
|         |   IDT (4096 bytes) located   |         |
|         |   within kernel DRAM space   |         |
|         +------------------------------+         |
|                                                  |
+--------------------------------------------------+ 0x00108000
|                                                  |
|       Kernel KFLASH (.text, .asm, .rodata)       |
|                 (32,768 bytes)                   |
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
+--------------------------------------------------+ 0x0000F000
|               Kernel Heap Metadata               |
|       (25600 bytes) + 3072 FOR ALIGN(4096)       |
+--------------------------------------------------+ 0x00008000
|                  (Unused Space)                  |
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