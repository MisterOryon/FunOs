# ATA Drives

ATA (Advanced Technology Attachment) is a standard interface for connecting storage such as hard drives and solid-state
drives to the motherboard.  
In OS kernels, ATA is also commonly referred to as IDE (Integrated Drive Electronics).  
ATA drives play an essential role by allowing the BIOS or UEFI to read the bootloader from the disk and enabling the
kernel to communicate with the disk.

## Communicating with ATA Drives

To communicate with an ATA drive, we use commands that are sent and responses that are read through the drive's control
registers.  
These registers are mapped to certain addresses in I/O space and accessed in assembly using the `in` and `out`
instructions.

In most cases, an operating system will abstract away these details by providing higher-level APIs for accessing storage
devices.

## LBA vs CHS

When working with storage, there are two ways to specify the location of data on the disk: LBA (Logical Block
Addressing) and CHS (Cylinder-Head-Sector).

Note that BIOS using interrupt `0x13` provides a method to use the CHS format.

### CHS

CHS addressing is an older method of addressing disks that involves specifying the Cylinder, Head, and Sector where data
is located.

- **Cylinder**: A vertical slice through the platters of a hard drive consisting of all the tracks that are at the same
  distance from the center of the platters.
- **Head**: Refers to the read/write head that accesses the platter surface where sectors are stored.
- **Sector**: A subdivision of a track on the disk (typically 512 bytes).

The main advantage of CHS is that it directly corresponds to the physical structure of a hard disk.
However, it is complex to work with and does not scale well with larger drives.

### LBA

LBA provides an abstract method to perform access.
Instead of using cylinders, heads, and sectors, data is accessed by specifying a simple block number starting from 0.

The main disadvantage of LBA is that it abstracts the physical layout of the disk, which can potentially lead to less
efficient data access patterns.  
Today all modern operating systems and BIOS/UEFI implementations use LBA for disk access.
