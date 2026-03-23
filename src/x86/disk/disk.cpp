//
// Created by oryon on 8/21/25.
//

#include "disk.h"

#include "io.h"

#include "config.h"
#include "memory.h"
#include "status.h"

using namespace funos::disk;

Disk gIdeDevices[4];

/**
 * Read sectors from an ATA device.
 *
 * @param lba Logical Block Address to start reading from.
 * @param totalSectors Number of sectors to read.
 * @param buffer Destination buffer for sector data.
 * @param secondaryChan Whether to use the secondary ATA channel (true) or primary (false).
 * @param slaveDevice Whether to read from the slave device (true) or master (false).
 * @return FUNOS_ALL_OK on success, -ETIMEDOUT if the device doesn't respond in time.
 */
static int readSector(
    const unsigned lba, const unsigned totalSectors, void* buffer,
    const bool secondaryChan, const bool slaveDevice
)
{
    const unsigned short ataPort = secondaryChan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;
    const unsigned short driveSelectBits = slaveDevice ? ATA_HDDEVSEL_DRV_SLAVE : ATA_HDDEVSEL_DRV_MASTER;
    const unsigned short lbaHighBits = ((lba >> ATA_LBA3_SHIFT) & 0x0F);

    unsigned short* bufferPtr = static_cast<unsigned short*>(buffer);
    unsigned char deviceStatus = 0;
    unsigned pollAttempts = 0;

    // Configure drive and send read command to controller.
    funos::io::outb(
        ataPort + ATA_REG_HDDEVSEL,
        (lbaHighBits | driveSelectBits)
    );
    funos::io::outb(ataPort + ATA_REG_SECCOUNT0, totalSectors);
    funos::io::outb(ataPort + ATA_REG_LBA0, (lba >> ATA_LBA0_SHIFT));
    funos::io::outb(ataPort + ATA_REG_LBA1, (lba >> ATA_LBA1_SHIFT));
    funos::io::outb(ataPort + ATA_REG_LBA2, (lba >> ATA_LBA2_SHIFT));
    funos::io::outb(ataPort + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    // Read each sector from the device.
    for (unsigned i = 0; i < totalSectors; i++)
    {
        pollAttempts = 0;

        // Poll until sector data is ready in the device buffer.
        deviceStatus = funos::io::inb(ataPort + ATA_REG_STATUS);
        while (!(deviceStatus & ATA_SR_DRQ))
        {
            if (pollAttempts++ > FUNOS_DISK_MAX_READ_ATTEMPTS) return -ETIMEDOUT;
            deviceStatus = funos::io::inb(ataPort + ATA_REG_STATUS);
        }

        // Transfer sector data from device.
        for (int j = 0; j < FUNOS_DISK_SECTOR_SIZE; j++)
        {
            *bufferPtr = funos::io::inw(ataPort + ATA_REG_DATA);
            bufferPtr++;
        }
    }

    return FUNOS_ALL_OK;
}

/**
 * Reads device identification data from an ATA device using the IDENTIFY command.
 *
 * @param secondaryChan Whether to use the secondary ATA channel (true) or primary (false).
 * @param slaveDevice Whether to query the slave device (true) or master (false).
 * @param isPresent Output parameter that indicates whether a device is present or not.
 * @param identifyBuffer Pointer to a buffer where the IDENTIFY data (256 words) will be stored if the device is present.
 * @return FUNOS_ALL_OK on success, -EIO on a device error, -ETIMEDOUT on timeout.
 */
static int readIndentify(
    const bool secondaryChan, const bool slaveDevice,
    bool* isPresent, unsigned short* identifyBuffer
)
{
    const unsigned short ataPort = secondaryChan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;
    const unsigned short driveSelectBits = slaveDevice ? ATA_HDDEVSEL_DRV_SLAVE : ATA_HDDEVSEL_DRV_MASTER;

    unsigned char deviceStatus = '\0';
    unsigned pollAttempts = 0;

    *isPresent = false;

    // Select the drive and issue the IDENTIFY command.
    funos::io::outb(ataPort + ATA_REG_HDDEVSEL, driveSelectBits);
    funos::io::outb(ataPort + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    // Check if a device is present (status register != 0).
    if (funos::io::inb(ataPort + ATA_REG_STATUS) == 0) return FUNOS_ALL_OK;


    // Poll until the ATA controller is ready to send IDENTIFY data.
    while (true)
    {
        deviceStatus = funos::io::inb(ataPort + ATA_REG_STATUS);

        if (deviceStatus & ATA_SR_ERR) return -EIO;
        if (!(deviceStatus & ATA_SR_BSY) && (deviceStatus & ATA_SR_DRQ)) break;

        if (pollAttempts++ > FUNOS_DISK_MAX_READ_ATTEMPTS) return -ETIMEDOUT;
    }

    for (int wordIndex = 0; wordIndex < 256; wordIndex++)
        identifyBuffer[wordIndex] = funos::io::inw(ataPort + ATA_REG_DATA);
    *isPresent = true;

    return FUNOS_ALL_OK;
}

/**
 * Parse model string from IDENTIFY data.
 *
 * @param identifyBuffer IDENTIFY response.
 * @param modelNumberBuffer Output buffer (must be at least 41 bytes).
 *
 * Note: ATA stores the model number as byte-swapped 16-bit words in words 27-46
 */
static void extractModelNumber(const unsigned short* identifyBuffer, char* modelNumberBuffer)
{
    for (int wordIndex = 0; wordIndex < 20; wordIndex++)
    {
        // little-endian, the bytes are flipped
        modelNumberBuffer[wordIndex * 2] = static_cast<char>(identifyBuffer[27 + wordIndex] >> 8);
        modelNumberBuffer[wordIndex * 2 + 1] = static_cast<char>(identifyBuffer[27 + wordIndex] & 0x00FF);
    }
    modelNumberBuffer[40] = 0; // terminate string.
}

/**
 * Check if the device supports the ATA/ATAPI-5 standard.
 *
 * @param identifyBuffer IDENTIFY response.
 * @return true if ATA/ATAPI-5 supported, false otherwise.
 */
static bool isATA5Supported(const unsigned short* identifyBuffer)
{
    // Bit 5 of word 80 indicates ATA/ATAPI-5 support
    return (identifyBuffer[80] & 0x20);
}

/**
 * Determine device type (ATA vs ATAPI) from signature registers.
 *
 * @param secondaryChan ATA channel to query.
 * @return ATA_REAL for ATA devices, ATA_PACKET for ATAPI devices.
 */
static disk_type_t detectDiskType(const bool secondaryChan)
{
    const unsigned short ataPort = secondaryChan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;

    const unsigned char sectorCount = funos::io::inb(ataPort + ATA_REG_SECCOUNT0);
    const unsigned char lbaLow = funos::io::inb(ataPort + ATA_REG_LBA0);
    const unsigned char lbaMid = funos::io::inb(ataPort + ATA_REG_LBA1);
    const unsigned char lbaHigh = funos::io::inb(ataPort + ATA_REG_LBA2);

    const unsigned long deviceSignature = sectorCount | (lbaLow << 8) | (lbaMid << 16) | (lbaHigh << 24);

    // See: https://wiki.osdev.org/ATAPI#Detecting.
    if (deviceSignature == ATA_ATAPI_SIG) return ATA_PACKET;
    return ATA_REAL;
}

int funos::disk::searchAndInitialize()
{
    int result = FUNOS_ALL_OK;

    unsigned short identifyBuffer[256];
    bool isPresent = false;
    Disk* currantDisk = nullptr;

    // Probe both ATA channels (primary, secondary) and both drive positions.
    for (int ataChannel = 0; ataChannel < 2; ataChannel++)
        for (int drivePosition = 0; drivePosition < 2; drivePosition++)
        {
            memory::memset(identifyBuffer, 0, sizeof(unsigned short) * 256);
            currantDisk = &gIdeDevices[ataChannel * 2 + drivePosition];

            result = readIndentify(ataChannel, drivePosition, &isPresent, identifyBuffer);
            if (result < 0)
            {
                // TODO:
                // We assume that if en error occur during reade the second channel of ATA PIC,
                // This is due to the absence of device that disable second channel.
                //
                // I future work we need to chack if the second channel of ATA PIC is available.
                if (ataChannel != 1 && result != -EIO) return result;
            }

            // No disk connects to this cable.
            if (!isPresent)
            {
                currantDisk->isPresent = false;
                continue;
            }

            if (!isATA5Supported(identifyBuffer)) return -EIO;

            currantDisk->type = detectDiskType(ataChannel);
            if (currantDisk->type == ATA_PACKET) return -EIO;

            currantDisk->isPresent = true;
            currantDisk->isSecondaryChan = ataChannel;
            currantDisk->isSlave = drivePosition;
            extractModelNumber(identifyBuffer, currantDisk->modelNumber);
            currantDisk->sectorSize = FUNOS_DISK_SECTOR_SIZE;
        }

    return FUNOS_ALL_OK;
}

Disk* funos::disk::getDisk(const unsigned index)
{
    if (index > 3) return nullptr;
    return &gIdeDevices[index];
}

int funos::disk::readBlock(const Disk* disk, const unsigned lba, const int totalSectors, void* buffer)
{
    if (disk == nullptr || buffer == nullptr) return -EINVAREG;
    if (disk->isPresent == false) return -EINVAREG;
    return readSector(lba, totalSectors, buffer, disk->isSecondaryChan, disk->isSlave);
}
