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