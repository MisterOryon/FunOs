//
// Created by oryon on 8/21/25.
//

#ifndef DISK_H
#define DISK_H

#include <stdbool.h>

#define ATA_IO_MASTER_REG          0x1F0 // Primary ATA channel base port
#define ATA_IO_SLAVE_REG           0x170 // Secondary ATA channel base port

#define ATA_HDDEVSEL_DRV_MASTER    0xE0 // Select master drive
#define ATA_HDDEVSEL_DRV_SLAVE     0xF0 // Select slave drive

#define ATA_ATAPI_SIG              0x010114EB // Device signature for ATAPI detection

/* LBA bit shift positions for 28-bit addressing */

#define ATA_LBA0_SHIFT             0
#define ATA_LBA1_SHIFT             8
#define ATA_LBA2_SHIFT             16
#define ATA_LBA3_SHIFT             24

/* Status */

#define ATA_SR_ERR                 0x01    // Error
#define ATA_SR_IDX                 0x02    // Index
#define ATA_SR_CORR                0x04    // Corrected data
#define ATA_SR_DRQ                 0x08    // Data request ready
#define ATA_SR_DSC                 0x10    // Drive seek complete
#define ATA_SR_DF                  0x20    // Drive write fault
#define ATA_SR_DRDY                0x40    // Drive ready
#define ATA_SR_BSY                 0x80    // Busy

/* Errors */

#define ATA_ER_AMNF                0x01    // No address mark
#define ATA_ER_TK0NF               0x02    // Track 0 not found
#define ATA_ER_ABRT                0x04    // Command aborted
#define ATA_ER_MCR                 0x08    // Media change request
#define ATA_ER_IDNF                0x10    // ID mark not found
#define ATA_ER_MC                  0x20    // Media changed
#define ATA_ER_UNC                 0x40    // Uncorrectable data
#define ATA_ER_BBK                 0x80    // Bad block

/* Commands */

#define ATA_CMD_READ_PIO           0x20
#define ATA_CMD_READ_PIO_EXT       0x24
#define ATA_CMD_READ_DMA           0xC8
#define ATA_CMD_READ_DMA_EXT       0x25
#define ATA_CMD_WRITE_PIO          0x30
#define ATA_CMD_WRITE_PIO_EXT      0x34
#define ATA_CMD_WRITE_DMA          0xCA
#define ATA_CMD_WRITE_DMA_EXT      0x35
#define ATA_CMD_CACHE_FLUSH        0xE7
#define ATA_CMD_CACHE_FLUSH_EXT    0xEA
#define ATA_CMD_PACKET             0xA0
#define ATA_CMD_IDENTIFY_PACKET    0xA1
#define ATA_CMD_IDENTIFY           0xEC

/* Register */

#define ATA_REG_DATA               0x00
#define ATA_REG_ERROR              0x01
#define ATA_REG_FEATURES           0x01
#define ATA_REG_SECCOUNT0          0x02
#define ATA_REG_LBA0               0x03
#define ATA_REG_LBA1               0x04
#define ATA_REG_LBA2               0x05
#define ATA_REG_HDDEVSEL           0x06
#define ATA_REG_COMMAND            0x07
#define ATA_REG_STATUS             0x07
#define ATA_REG_SECCOUNT1          0x08
#define ATA_REG_LBA3               0x09
#define ATA_REG_LBA4               0x0A
#define ATA_REG_LBA5               0x0B
#define ATA_REG_CONTROL            0x0C
#define ATA_REG_ALTSTATUS          0x0C
#define ATA_REG_DEVADDRESS         0x0D

/* disk_type_t represents the type of ATA device */
typedef enum : unsigned
{
    // ATA hard disk drive.
    ATA_REAL = 1,
    // ATAPI device (CD/DVD, tape drive).
    ATA_PACKET = 2
} disk_type_t;

/* disk represents an IDE/ATA device */
struct disk
{
    bool present; // Device is detected and initialized.
    bool secondary_chan; // Connected to the secondary ATA channel (true) or primary (false).
    bool is_slave; // Device is slave (true) or master (false) on the channel.

    disk_type_t type; // Device type: ATA_REAL for HDD, ATA_PACKET for ATAPI.
    char model_number[41]; // Null-terminated model string.

    int sector_size; // Bytes per sector.
};

/**
 * @brief Detect and initialize all IDE/ATA devices.
 *
 * This function searches for and initializes all IDE/ATA devices connected to the system.
 * It scans the primary and secondary ATA channels for devices and initializes them if detected.
 *
 * @return FUNOS_ALL_OK on success, otherwise an error code.
 */
int disk_search_and_init();

/**
 * @brief Retrieve a disk device by index.
 *
 * This function retrieves a disk device by its index.
 * The index corresponds to the order in which devices were detected and initialized during disk_search_and_init().
 *
 * @param index Device index.
 * @return Pointer to the disk structure if found, NULL otherwise.
 */
struct disk* disk_get(const unsigned index);

/**
 * @brief Read sectors from a disk device.
 *
 * @param idisk Device handle.
 * @param lba Logical block address.
 * @param total_sectors Number of sectors to read.
 * @param buffer Buffer to store the read data.
 * @return FUNOS_ALL_OK on success, otherwise an error code.
 */
int disk_read_block(const struct disk* idisk, const unsigned lba, const int total_sectors, void* buffer);

#endif //DISK_H
