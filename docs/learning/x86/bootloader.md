# Bootloader

A bootloader is a small program responsible for loading the kernel. Bootloaders are generally small in size.

When a computer boots, it starts in a compatibility mode called "real mode."  
This mode provides access to up to one megabyte of memory and runs 16-bit code.

The responsibility of our bootloader is to transition the processor into "protected mode," which provides access to four
gigabytes of memory and runs 32-bit code.  
After entering protected mode, the job of the bootloader is to load the kernel into memory and transfer execution to the
kernel.

Bootloaders generally use the functions provided by the BIOS to assist in their tasks.
