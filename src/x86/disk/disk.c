//
// Created by oryon on 8/21/25.
//

#include "disk.h"

#include <config.h>
#include <kheap.h>
#include <memory.h>
#include <status.h>

#include "io/io.h"

struct disk ide_devices[4];

/**
 * Read sectors from an ATA device.
 *
 * @param lba Logical Block Address to start reading from.
 * @param total_sectors Number of sectors to read.
 * @param buffer Destination buffer for sector data.
 * @param secondary_chan Whether to use the secondary ATA channel (true) or primary (false).
 * @param slave_device Whether to read from the slave device (true) or master (false).
 * @return FUNOS_ALL_OK on success, -ETIMEDOUT if the device doesn't respond in time.
 */
static int disk_read_sector(
    const unsigned lba, const unsigned total_sectors, void* buffer,
    const bool secondary_chan, const bool slave_device
)
{
    const unsigned short ata_port = secondary_chan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;
    const unsigned short drive_select_bits = slave_device ? ATA_HDDEVSEL_DRV_SLAVE : ATA_HDDEVSEL_DRV_MASTER;
    const unsigned short lba_high_bits = ((lba >> ATA_LBA3_SHIFT) & 0x0F);

    int res = FUNOS_ALL_OK;

    unsigned short* buffer_ptr = buffer;
    unsigned char device_status = 0;
    unsigned poll_attempts = 0;

    // Configure drive and send read command to controller.
    io_outb(
        ata_port + ATA_REG_HDDEVSEL,
        (lba_high_bits | drive_select_bits)
    );
    io_outb(ata_port + ATA_REG_SECCOUNT0, total_sectors);
    io_outb(ata_port + ATA_REG_LBA0, (lba >> ATA_LBA0_SHIFT));
    io_outb(ata_port + ATA_REG_LBA1, (lba >> ATA_LBA1_SHIFT));
    io_outb(ata_port + ATA_REG_LBA2, (lba >> ATA_LBA2_SHIFT));
    io_outb(ata_port + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    // Read each sector from the device.
    for (int b = 0; b < total_sectors; b++)
    {
        poll_attempts = 0;

        // Poll until sector data is ready in the device buffer.
        device_status = io_inb(ata_port + ATA_REG_STATUS);
        while (!(device_status & ATA_SR_DRQ))
        {
            if (poll_attempts++ > FUNOS_DISK_MAX_READ_ATTEMPTS) return -ETIMEDOUT;
            device_status = io_inb(ata_port + ATA_REG_STATUS);
        }

        // Transfer sector data from device.
        for (int i = 0; i < FUNOS_DISK_SECTOR_SIZE; i++)
        {
            *buffer_ptr = io_inw(ata_port + ATA_REG_DATA);
            buffer_ptr++;
        }
    }

    return res;
}

/**
 * Send IDENTIFY command and retrieve device capabilities.
 *
 * @param secondary_chan Whether to use the secondary ATA channel (true) or primary (false).
 * @param slave_device Whether to query the slave device (true) or master (false).
 * @param identify_buffer Output parameter pointer to 256-word identify data, must be freed by caller.
 * @return FUNOS_ALL_OK on success, -EIO on device error, -ETIMEDOUT on timeout, -ENOMEN if memory allocation fails.
 */
static int disk_read_indentify(
    const bool secondary_chan, const bool slave_device,
    unsigned short** identify_buffer
)
{
    const unsigned short ata_port = secondary_chan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;
    const unsigned short drive_select_bits = slave_device ? ATA_HDDEVSEL_DRV_SLAVE : ATA_HDDEVSEL_DRV_MASTER;

    unsigned char device_status = '\0';
    unsigned poll_attempts = 0;

    // Select the drive and issue the IDENTIFY command.
    io_outb(ata_port + ATA_REG_HDDEVSEL, drive_select_bits);
    io_outb(ata_port + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    // Check if a device is present (status register != 0).
    if (io_inb(ata_port + ATA_REG_STATUS) == 0) return FUNOS_ALL_OK;

    // Poll until the ATA controller is ready to send IDENTIFY data.
    while (1)
    {
        device_status = io_inb(ata_port + ATA_REG_STATUS);

        if (device_status & ATA_SR_ERR) return -EIO;
        if (!(device_status & ATA_SR_BSY) && (device_status & ATA_SR_DRQ)) break;

        if (poll_attempts++ > FUNOS_DISK_MAX_READ_ATTEMPTS) return -ETIMEDOUT;
    }

    // Allocate buffer and read 256 16-bit words of IDENTIFY data.
    *identify_buffer = kernel_malloc(sizeof(unsigned short) * 256);
    if (*identify_buffer == NULL) return -ENOMEN;

    for (int word_idx = 0; word_idx < 256; word_idx++) (*identify_buffer)[word_idx] = io_inw(ata_port + ATA_REG_DATA);

    return FUNOS_ALL_OK;
}

/**
 * Parse model string from IDENTIFY data.
 *
 * @param identify_buffer IDENTIFY response.
 * @param model_number_buffer Output buffer (must be at least 41 bytes).
 *
 * Note: ATA stores the model number as byte-swapped 16-bit words in words 27-46
 */
static void disk_extract_model_number(const unsigned short* identify_buffer, char* model_number_buffer)
{
    for (int word_idx = 0; word_idx < 20; word_idx++)
    {
        // little-endian, the bytes are flipped
        model_number_buffer[word_idx * 2] = (char)(identify_buffer[27 + word_idx] >> 8);
        model_number_buffer[word_idx * 2 + 1] = (char)(identify_buffer[27 + word_idx] & 0x00FF);
    }
    model_number_buffer[40] = 0; // terminate string.
}

/**
 * Check if the device supports the ATA/ATAPI-5 standard.
 *
 * @param identify_buffer IDENTIFY response.
 * @return true if ATA/ATAPI-5 supported, false otherwise.
 */
static bool disk_supports_ata_atapi_5(const unsigned short* identify_buffer)
{
    // Bit 5 of word 80 indicates ATA/ATAPI-5 support
    return (identify_buffer[80] & 0x20);
}

/**
 * Determine device type (ATA vs ATAPI) from signature registers.
 *
 * @param secondary_chan ATA channel to query.
 * @return ATA_REAL for ATA devices, ATA_PACKET for ATAPI devices.
 */
static disk_type_t get_disk_type(const bool secondary_chan)
{
    const unsigned short ata_port = secondary_chan ? ATA_IO_SLAVE_REG : ATA_IO_MASTER_REG;

    const unsigned char sector_count = io_inb(ata_port + ATA_REG_SECCOUNT0);
    const unsigned char lba_low = io_inb(ata_port + ATA_REG_LBA0);
    const unsigned char lba_mid = io_inb(ata_port + ATA_REG_LBA1);
    const unsigned char lba_high = io_inb(ata_port + ATA_REG_LBA2);

    const unsigned long device_signature = sector_count | (lba_low << 8) | (lba_mid << 16) | (lba_high << 24);

    // See: https://wiki.osdev.org/ATAPI#Detecting.
    if (device_signature == ATA_ATAPI_SIG) return ATA_PACKET;
    return ATA_REAL;
}

/**
 * Detect and initialize all IDE/ATA devices.
 */
int disk_search_and_init()
{
    int res = FUNOS_ALL_OK;

    unsigned short* identify_buffer = NULL;
    struct disk* currant_disk = NULL;

    // Probe both ATA channels (primary, secondary) and both drive positions.
    for (int ata_channel = 0; ata_channel < 2; ata_channel++)
        for (int drive_position = 0; drive_position < 2; drive_position++)
        {
            identify_buffer = NULL;
            currant_disk = &ide_devices[ata_channel * 2 + drive_position];

            res = disk_read_indentify(ata_channel, drive_position, &identify_buffer);
            if (res < 0)
                // TODO:
                // We assume that if en error occur during reade the second channel of ATA PIC,
                // This is due to the absence of device that disable second channel.
                //
                // I future work we need to chack if the second channel of ATA PIC is available.
                if (ata_channel != 1 && res != -EIO) return res;

            // No disk connects to this cable.
            if (identify_buffer == NULL)
            {
                currant_disk->present = false;
                continue;
            }
            if (!disk_supports_ata_atapi_5(identify_buffer)) goto cleanup_error;

            currant_disk->present = true;
            currant_disk->secondary_chan = ata_channel;
            currant_disk->is_slave = drive_position;
            currant_disk->type = get_disk_type(ata_channel);
            if (currant_disk->type == ATA_PACKET) goto cleanup_error;
            disk_extract_model_number(identify_buffer, currant_disk->model_number);
            currant_disk->sector_size = FUNOS_DISK_SECTOR_SIZE;

            kernel_free(identify_buffer);
            continue;

        cleanup_error:
            kernel_free(identify_buffer);
            return -EIO;
        }

    return res;
}

/**
 * Retrieve a disk device by index.
 */
struct disk* disk_get(const unsigned index)
{
    if (index > 3) return NULL;
    return &ide_devices[index];
}

/**
 * Read sectors from a disk device.
 */
int disk_read_block(const struct disk* idisk, const unsigned lba, const int total_sectors, void* buffer)
{
    if (idisk == NULL || buffer == NULL) return -EINVAREG;
    if (idisk->present == false) return -EINVAREG;
    return disk_read_sector(lba, total_sectors, buffer, idisk->secondary_chan, idisk->is_slave);
}
