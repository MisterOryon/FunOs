//
// Created by oryon on 8/21/25.
//

#ifndef DISK_H
#define DISK_H

#include <stdint.h>

namespace funos
{
    namespace disk
    {
        constexpr uint16_t ATA_IO_MASTER_REG = 0x1F0; // Primary ATA channel base port
        constexpr uint16_t ATA_IO_SLAVE_REG = 0x170; // Secondary ATA channel base port

        constexpr uint8_t ATA_HDDEVSEL_DRV_MASTER = 0xE0; // Select master drive
        constexpr uint8_t ATA_HDDEVSEL_DRV_SLAVE = 0xF0; // Select slave drive

        constexpr uint32_t ATA_ATAPI_SIG = 0x010114EB; // Device signature for ATAPI detection


        /* LBA bit shift positions for 28-bit addressing */

        constexpr uint8_t ATA_LBA0_SHIFT = 0;
        constexpr uint8_t ATA_LBA1_SHIFT = 8;
        constexpr uint8_t ATA_LBA2_SHIFT = 16;
        constexpr uint8_t ATA_LBA3_SHIFT = 24;

        /* Status */

        constexpr uint8_t ATA_SR_ERR = 0x01; // Error
        constexpr uint8_t ATA_SR_IDX = 0x02; // Index
        constexpr uint8_t ATA_SR_CORR = 0x04; // Corrected data
        constexpr uint8_t ATA_SR_DRQ = 0x08; // Data request ready
        constexpr uint8_t ATA_SR_DSC = 0x10; // Drive seek complete
        constexpr uint8_t ATA_SR_DF = 0x20; // Drive write fault
        constexpr uint8_t ATA_SR_DRDY = 0x40; // Drive ready
        constexpr uint8_t ATA_SR_BSY = 0x80; // Busy


        /* Errors */

        constexpr uint8_t ATA_ER_AMNF = 0x01; // No address mark
        constexpr uint8_t ATA_ER_TK0NF = 0x02; // Track 0 not found
        constexpr uint8_t ATA_ER_ABRT = 0x04; // Command aborted
        constexpr uint8_t ATA_ER_MCR = 0x08; // Media change request
        constexpr uint8_t ATA_ER_IDNF = 0x10; // ID mark not found
        constexpr uint8_t ATA_ER_MC = 0x20; // Media changed
        constexpr uint8_t ATA_ER_UNC = 0x40; // Uncorrectable data
        constexpr uint8_t ATA_ER_BBK = 0x80; // Bad block


        /* Commands */

        constexpr uint8_t ATA_CMD_READ_PIO = 0x20;
        constexpr uint8_t ATA_CMD_READ_PIO_EXT = 0x24;
        constexpr uint8_t ATA_CMD_READ_DMA = 0xC8;
        constexpr uint8_t ATA_CMD_READ_DMA_EXT = 0x25;
        constexpr uint8_t ATA_CMD_WRITE_PIO = 0x30;
        constexpr uint8_t ATA_CMD_WRITE_PIO_EXT = 0x34;
        constexpr uint8_t ATA_CMD_WRITE_DMA = 0xCA;
        constexpr uint8_t ATA_CMD_WRITE_DMA_EXT = 0x35;
        constexpr uint8_t ATA_CMD_CACHE_FLUSH = 0xE7;
        constexpr uint8_t ATA_CMD_CACHE_FLUSH_EXT = 0xEA;
        constexpr uint8_t ATA_CMD_PACKET = 0xA0;
        constexpr uint8_t ATA_CMD_IDENTIFY_PACKET = 0xA1;
        constexpr uint8_t ATA_CMD_IDENTIFY = 0xEC;


        /* Register */

        constexpr uint8_t ATA_REG_DATA = 0x00;
        constexpr uint8_t ATA_REG_ERROR = 0x01;
        constexpr uint8_t ATA_REG_FEATURES = 0x01;
        constexpr uint8_t ATA_REG_SECCOUNT0 = 0x02;
        constexpr uint8_t ATA_REG_LBA0 = 0x03;
        constexpr uint8_t ATA_REG_LBA1 = 0x04;
        constexpr uint8_t ATA_REG_LBA2 = 0x05;
        constexpr uint8_t ATA_REG_HDDEVSEL = 0x06;
        constexpr uint8_t ATA_REG_COMMAND = 0x07;
        constexpr uint8_t ATA_REG_STATUS = 0x07;
        constexpr uint8_t ATA_REG_SECCOUNT1 = 0x08;
        constexpr uint8_t ATA_REG_LBA3 = 0x09;
        constexpr uint8_t ATA_REG_LBA4 = 0x0A;
        constexpr uint8_t ATA_REG_LBA5 = 0x0B;
        constexpr uint8_t ATA_REG_CONTROL = 0x0C;
        constexpr uint8_t ATA_REG_ALTSTATUS = 0x0C;
        constexpr uint8_t ATA_REG_DEVADDRESS = 0x0D;


        /* disk_type_t represents the type of ATA device */
        typedef enum : unsigned
        {
            // ATA hard disk drive.
            ATA_REAL = 1,
            // ATAPI device (CD/DVD, tape drive).
            ATA_PACKET = 2
        } disk_type_t;

        /* disk represents an IDE/ATA device */
        struct Disk
        {
            bool isPresent; // Device is detected and initialized.
            bool isSecondaryChan; // Connected to the secondary ATA channel (true) or primary (false).
            bool isSlave; // Device is slave (true) or master (false) on the channel.

            disk_type_t type; // Device type: ATA_REAL for HDD, ATA_PACKET for ATAPI.
            char modelNumber[41]; // Null-terminated model string.

            int sectorSize; // Bytes per sector.
        };

        /**
         * @brief Detect and initialize all IDE/ATA devices.
         *
         * This function searches for and initializes all IDE/ATA devices connected to the system.
         * It scans the primary and secondary ATA channels for devices and initializes them if detected.
         *
         * @return FUNOS_ALL_OK on success, otherwise an error code.
         */
        int searchAndInitialize();

        /**
         * @brief Retrieve a disk device by index.
         *
         * This function retrieves a disk device by its index.
         * The index corresponds to the order in which devices were detected and initialized during disk_search_and_init().
         *
         * @param index Device index.
         * @return Pointer to the disk structure if found, NULL otherwise.
         */
        Disk* getDisk(unsigned index);

        /**
         * @brief Read sectors from a disk device.
         *
         * @param disk Device handle.
         * @param lba Logical block address.
         * @param totalSectors Number of sectors to read.
         * @param buffer Buffer to store the read data.
         * @return FUNOS_ALL_OK on success, otherwise an error code.
         */
        int readBlock(const Disk* disk, unsigned lba, int totalSectors, void* buffer);
    }
}

#endif //DISK_H
