# PCI (Peripheral Component Interconnect)

PCI is a standard for connecting a motherboard to the peripheral components of a computer system.

## IDE Controller

The PCI IDE controller allows communication between the CPU and IDE devices.
IDE is a standard for connecting a motherboard to storage devices like hard drives and CD-ROM drives.
The PCI IDE controllers manage the transfer of data between the CPU and IDE devices by converting the parallel data
from IDE devices into a serial data stream that the PCI bus can handle and vice versa.

The PCI IDE controller can directly access memory to read and write data, bypassing the CPU and reducing its
workload.
This DMA (Direct Memory Access) can significantly improve system performance, especially when dealing with large
data transfers.
Note that the PCI IDE controller was a standard in earlier systems but has been replaced by more advanced technologies
such as SATA (Serial Advanced Technology Attachment).

### PCI Drive Types

There are several types of PCI drives:

- **IDE (Integrated Drive Electronics)**:
  IDE (also known as ATA) is one of the oldest drive types.
  It uses parallel signaling to transfer data and is commonly associated with older HDDs and optical drives.

- **SCSI (Small Computer System Interface)**:
  SCSI is a standard for connecting and transferring data between computers and peripheral devices.
  SCSI was popular due to its high performance and ability to connect multiple devices on a single bus.
  Like IDE, SCSI uses parallel signaling to transfer data, but includes additional features such as greater cable length
  and support for more connected devices.

- **SATA (Serial ATA)**:
  SATA is the successor to the IDE/ATA standard.
  SATA uses serial signaling, which can result in faster data transfer rates compared to parallel signaling due to
  reduced crosstalk and signal interference.

- **NVMe (Non-Volatile Memory Express)**:
  NVMe is a newer standard for SSD drives.
  Unlike other standards, NVMe uses a PCIe (Peripheral Component Interconnect Express) interface that is a more modern
  and faster successor to the PCI bus.
  NVMe drives offer significantly higher performance compared to SATA SSDs.

### Master Vs Slave

The terms "master" and "slave" or "primary" and "secondary" refer to the way in which two devices share a single cable
connection to the motherboard.
The master is the primary device on the cable, and it often has priority in communication with the motherboard.
In general, if you have only one device on the cable, it should be configured as the master.
The slave is the secondary device on the cable, it communicates with the motherboard only when the master device isn't
using the data bus.
It's important to note that in most modern systems such as SATA, each device has its own cable, eliminating the need for
master/slave distinctions.
In terms of performance and specifications, the master and slave drives are identical.
But in ATA hard drives, the master drive is typically the drive from which the system boots, and the slave is used as
additional storage space.
The distinction between master and slave only dictates their role during the system boot sequence.

### LBA (Logical Block Addressing)

In LBA, the storage space on a drive is divided into blocks of fixed size.
On most hard drives, the block size is 512 bytes.
When an OS wants to read or write data, it specifies the LBA of the block where the data should be read from or written
to.
The controller of the storage device handles the translation of logical addresses into physical addresses on the
device where the operation will take place.
LBA has several advantages: it is easier to implement and more efficient by avoiding limitations and complexities
associated with the physical layout of storage devices.

### I/O Ports for Hard Drives

The I/O ports in the ATA play a crucial role in communication:

- **Port 0x1F6 (Drive/Head Register or Device Register)**:
  This port is used to select the drive and the head.
  The upper four bits of the port are used to select the head, and the lower bits are used for drive selection and other
  operations.
  Send 0xE0 for the master or 0xF0 for the slave.
- **Port 0x1F2 (Sector Count Register)**:
  This port is used to specify the number of sectors to read or write.
- **Ports 0x1F3, 0x1F4, and 0x1F5 (LBA Low, LBA Mid, and LBA High Registers)**:
  These ports are used to specify the LBA of the sector to read or write.
  The LBA is a 28-bit value which is distributed across these three ports and the lower 4 bits of port 0x1F6.
- **Port 0x1F7 (Status/Command Register)**:
  This port has two purposes: it provides the status of the disk controller when read and allows commands to be sent to
  the controller when written.
  Status bits include BUSY, DRDY (Drive Ready), DF (Drive Fault), and ERR (Error).

The slave drive works as the same as the master drive but with a different port base address, typically 0x170.
To select the slave drive, you would write 0xF0 to port 0x1F6 or 0x176.

### Reading In C

```c
outb(0x1F6, (lba >> 24) | 0xE0);
outb(0x1F2, total_nb_sectors);
outb(0x1F3, lba & 0xFF);
outb(0x1F4, lba >> 8);
outb(0x1F5, lba >> 16);
outb(0x1F7, 0x20); // Read command

for (int i = 0; i < total_nb_sectors; i++) {
    char c = insb(0x1F7);
    while (!(c & 0x80)) { // Wait until the disk is ready
        c = insb(0x1F7);
    }
    
    for (int j = 0; j < 256; j++) {
        *ptr = insw(0x1F0); // Read two bytes at a time
        ptr++;
    }
}
```