# Kernel Startup

The first function that the bootloader must call is `_start` in `kernel.asm`.
This function ensures that A20 and segment registers are set up correctly.
After that, it sets up the master PIC with interrupt offset 0x20 to enable it to handle interrupts.

Note that FunOs assumes that:

- Entry 0x08 in GDT is code segment and 0x10 is data segment.
- The processor is already in protected mode.

After `_start` has setup the environment, it calls `kernel_main` in `kernel.c`.
The `kernel_main` calls `idt_initialize`, `display_initialize`, `kernel_heap_init` and `kernel_paging_init` to complete
the setup of FunOs.

## IDT

The `idt_initialize` function sets up basic handlers for x86 interruption numbers 0 to 30.
It also sets up handlers for PIC IRQ 1: Keyboard controller interrupt.

Note that if it is an exception interruption, the handler prints a kernel panic message and stops the kernel execution.

## Display

The `display_initialize` function sets up the VGA text mode and clears all content shown on screen.
After that kernel developers can use `console_write` functions to print messages in the screen.

## Heap

The `kernel_heap_init` function sets up a kernel heap of 104,857,600 bytes (100MB) between memory
addresses 0x01000000 and 0x7400000.
After that, kernel developers can use `kernel_malloc` and `kernel_free` functions for dynamic memory allocation.

## Paging

The `kernel_paging_init` function configures the kernel memory paging structure according to the following layout:

1. **Real Mode Memory**:
    - Sets the first 1MB of memory inherited from real mode to read-only.
    - Exceptions: Kernel heap metadata (0x00008000-0x0000F000) and video memory (0x000B8000-0x000C0000) to read/write.

2. **Kernel Binary Memory**:
    - .text, .asm, and .rodata sections (0x00100000-0x00108000) to read-only.
    - .data and .bss sections (0x00108000-0x00118000) to read/write.

3. **Kernel Stack Memory**:
    - Stack Guard (0x00118000-0x0011A000) to read-only.
    - Stack (0x0011A000-0x00400000) read/write

4. **Kernel Heap**:
    - Allocates 25 page tables (100 MB of memory) for the kernel heap (0x01000000-0x07400000) read/write
