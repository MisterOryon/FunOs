# Kernel Startup

The first function that the bootloader must call is `_start` in `kernel.asm`.
This function ensures that A20 and segment registers are set up correctly.

Note that FunOs assumes that:

- Entry 0x08 in GDT is code segment and 0x10 is data segment.
- The processor is already in protected mode.

After `_start` has setup the environment, it calls `kernel_main` in `kernel.c`.
The `kernel_main` calls `idt_initialize` and `display_initialize` to complete the setup of FunOs.

## IDT

The `idt_initialize` function sets up basic handlers for x86 interruption numbers 0 to 30.
When an interruption is triggered, the handler prints a message corresponding to the interruption in the screen.
If it is an exception interruption, the handler prints a kernel panic message and stops the kernel execution.

## Display

The `display_initialize` function sets up the VGA text mode and clears all content shown on screen.
After that kernel developers can use `console_write` functions to print messages in the screen.