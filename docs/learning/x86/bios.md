# BIOS

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
