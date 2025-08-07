# Boot Sequence

The FunOs bootloader is responsible for initializing the computer and preparing it to run the FunOs kernel.
This process consists of the following key steps:

1. **BIOS Segment Register Verification**  
   The bootloader ensures that the BIOS has correctly configured the segment registers to `0x00`.

2. **Global Descriptor Table (GDT) Configuration**
    - The GDT is set up with two segments:
        - **CODE_SEG**: Code segment descriptor.
        - **DATA_SEG**: Data segment descriptor.
    - Both segments span the entire 32-bit memory address space (from the start of memory to 4 GB).

3. **Processor Mode Transition**
    - The bootloader switches the CPU from Real Mode to Protected Mode.
    - This is accomplished by enabling bit 0 in Control Register 0 (CR0).

4. **A20 Line Activation**
    - The A20 address line is enabled to allow access to memory beyond the first 1 MB.
    - This is necessary to support 32-bit addressable memory in Protected Mode.

5. **Segment Registers Update**
    - The segment registers are updated to use the newly configured GDT entries for `CODE_SEG` and `DATA_SEG`.

6. **Kernel Loading**
    - The bootloader reads 98,304 bytes (192 sectors, starting at sector 2) from the master disk.
    - These are loaded into memory at address `0x00100000` to prepare the kernel for execution.

7. **Execute FunOs kernel**