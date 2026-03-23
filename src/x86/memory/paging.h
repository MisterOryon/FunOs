//
// Created by oryon on 7/26/25.
//

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

namespace funos
{
    namespace memory
    {
        namespace mmu
        {
            /* Page attribute flag bits for x86 paging entries. */

            // PCD bit: Disables caching for the page.
            constexpr uint8_t PAGING_CACHE_DISABLED = 0b00010000;
            // PWT bit: Enables write-through caching policy.
            constexpr uint8_t PAGING_WRITE_THROUGH = 0b00001000;
            // U/S bit: Allows user-mode access (1) vs supervisor-only (0).
            constexpr uint8_t PAGING_ACCESS_FROM_ALL = 0b00000100;
            // R/W bit: Allows write access (1) vs read-only (0).
            constexpr uint8_t PAGING_IS_WRITEABLE = 0b00000010;
            // P bit: Page is present in memory (1) vs not present (0).
            constexpr uint8_t PAGING_IS_PRESENT = 0b00000001;

            /* Custom flag for x86 paging entries. */

            // Unset bit: Indicate that the entry is unset.
            constexpr uint16_t PAGING_UNSET = 0b001000000000;

            /* Constants defining the x86 paging structure dimensions. */

            constexpr uint16_t PAGING_TOTAL_ENTRIES_PER_TABLE = 1024;
            constexpr uint16_t PAGING_PAGE_SIZE = 4096;

            /**
             * Since the lower 12 bits of a page entry are used for flags, this mask isolates the upper 20 bits
             * which contain the actual physical memory address.
             */
            constexpr uint32_t PAGING_BASE_ADDR_MASK = 0xFFFFF000;

            /**
             * @brief Represents a chunk of the paging mechanism.
             *
             * This structure is part of the paging system and serves as a container for managing directory
             * entries associated with memory paging.
             */
            struct Chunk
            {
                uint32_t* pageDirectory; // Pointer to a directory entry structure used in the paging system.
            };

            /**
             * @brief Switches the current paging directory to a new one.
             *
             * @param chunk A pointer to the  chunk structure for which hold the page directory to switch to.
             *              The directory must already be set up with the required paging configurations.
             */
            void pagingSwitch(const Chunk* chunk);

            /**
             * @brief Enables paging in the processor.
             *
             * This function activates paging, allowing virtual memory to be mapped to physical memory.
             */
            void pagingEnable();

            /**
             * @brief Checks if a given address is aligned to the page size.
             *
             * This function determines whether the provided address adheres to the alignment requirements
             * of the paging system, which requires addresses to be multiples of the page size.
             *
             * @param addr A pointer to the memory address to check for alignment.
             * @return True if the address is aligned to the page size, otherwise false.
             */
            bool isPageAligned(const void* addr);

            /**
             * TODO comments
             * @brief Initializes a page directory for a memory chunk.
             *
             * This function sets up a page directory for the given chunk in the memory paging system.
             * It allocates and initializes the root page directory but deliberately marks its entries as undefined
             * to optimize memory usage.
             *
             * @note Page tables associated with directory entries are not allocated to conserve memory.
             * TODO :
             * Before using paging_set_page_mapping(), you must first configure the directory entry with
             * paging_set_directory_entry(), which will allocate the corresponding table page.
             *
             * @param chunk Pointer to the chunk structure for which the page directory is being initialized.
             * @return Returns an integer status code.
             *         0 on success, or an error code if allocation fails or if the input parameter is invalid.
             */
            int initializeChunk(Chunk* chunk);

            /**
             * TODO comments
             * @brief Completely destroys a chunk and all associated resources.
             *
             * This function performs a cleanup of all paging structures owned by a chunk.
             * This is typically used when a memory context is destroyed to prevent memory leaks.
             *
             * @param chunk A pointer to the chunk structure which should be destroyed.
             *              After this call, the chunk's structures will be invalid and should not be used.
             * @return Returns an integer status code.
             *         0 on success, or an error code if the provided parameters are invalid.
             */
            int freeChunk(const Chunk* chunk);

            /**
             * @brief Configures a page directory entry and allocates its associated page table.
             *
             * This function sets up a page directory entry for the specified virtual address with the specified flags.
             * If there isn't a page table associated with the page directory entry, this function allocates one before
             * configuring the page directory entry.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddr The virtual address whose page directory entry needs to be configured.
             *                    The address is used to determine which page directory entry to modify.
             * @param flags Control bits that define the properties of the page directory entry (present, writable, etc.).
             * @return Returns an integer status code.
             *         0 on success, or an error code if memory allocation fails or if parameters are invalid.
             */
            int initializePageDirectoryEntry(const Chunk* chunk, const void* virtualAddr, uint8_t flags);

            /**
             * @brief Configures multiple page directory entries for a virtual address range.
             *
             * This function setting up all page directory entries that cover the specified virtual address range.
             * For each page directory entry in the range, it allocates a page table if no one is already associated
             * before configuring the page directory entry with the provided flags.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddrStart The starting virtual address of the range to configure.
             *                         Must be aligned to the directory boundary for optimal behavior.
             * @param virtualAddrEnd The ending virtual address (exclusive) of the range to configure.
             * @param flags Control bits to apply to all page directory entries in the range (present, writable, etc.).
             * @return Returns an integer status code.
             *         0 if all directory entries were successfully configured, or an error code if memory allocation
             *         fails or if parameters are invalid
             */
            int initializePageDirectoryEntries(const Chunk* chunk,
                                               const void* virtualAddrStart, const void* virtualAddrEnd,
                                               uint8_t flags);

            /**
             * @brief Maps a virtual address to a physical address with specific access control flags.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddr The virtual address to be mapped.
             * @param physicalAddr The physical address to map to.
             * @param flags Bit flags controlling page properties (read/write permissions, user/supervisor mode, etc.).
             * @return Returns an integer status code.
             *         0 on success, or an error code if parameters are invalid or if page directory entry is not
             *         properly configured.
             */
            int setPageMapping(const Chunk* chunk,
                               const void* virtualAddr, const void* physicalAddr,
                               uint8_t flags);


            /**
             * @brief Maps a range of virtual addresses to physical addresses with specific access control flags.
             *
             * This function establishes multiple page mappings between a range of virtual addresses and the
             * corresponding range of physical addresses.
             * It iterates through the specified address range and sets up individual page mappings with consistent flags.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddrStart The starting virtual address of the range to map.
             * @param physicalAddrStart The starting physical address of the range to map to.
             * @param virtualAddrEnd The ending virtual address of the range (exclusive).
             * @param flags Bit flags controlling page properties for all mapped pages in the range
             *              (read/write permissions, user/supervisor mode, etc.).
             * @return Returns an integer status code.
             *         0 on success or an error code if page mapping fails or if address ranges are invalid
             *         or if once page directory entry is not properly configured.
             */
            int setPagesMapping(const Chunk* chunk,
                                const void* virtualAddrStart, const void* physicalAddrStart,
                                const void* virtualAddrEnd,
                                uint8_t flags);

            /**
             * @brief Translates a virtual address to its physical address and retrieves its page flags.
             *
             * This function performs virtual-to-physical address translation by traversing the page directory
             * and page tables within the specified chunk.
             * It retrieves both the physical address that corresponds to the provided virtual address
             * and the memory flags associated with the mapping.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddr The virtual address to be translated.
             * @param physicalAddr Pointer where the resolved physical address will be stored.
             * @param flags Pointer where the page flags will be stored (permissions, caching attributes, etc.).
             * @return Returns an integer status code.
             *         0 if the translation is successful, or an error code if the page table is not present or if
             *         the provided parameters are invalid.
             */
            int getPageMapping(const Chunk* chunk,
                               const void* virtualAddr, uint32_t* physicalAddr,
                               uint8_t* flags);

            /**
             * @brief Retrieves the page directory entry that controls mapping for the specified virtual address.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddr The virtual address whose page directory entry should be examined.
             * @param isUnset Pointer to a boolean that will be set to true if the directory entry is not present
             *                or false if it is properly configured.
             * @param flags Pointer to a variable where the page directory entry's flags will be stored (permissions, etc.).
             *              The flags are only meaningful if *isUnset is false.
             * @return Returns an integer status code.
             *         0 on success (regardless of whether the entry exists) or an error code if the provided
             *         parameters are invalid.
             */
            int getPageDirectoryEntryAttributes(const Chunk* chunk,
                                                const void* virtualAddr,
                                                bool* isUnset, uint8_t* flags);

            /**
             * @brief Marks a page directory entry as unset and frees its associated page table.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddr The virtual address whose directory entry should be unset.
             *                     All pages mapped within this directory entry's range will become inaccessible.
             * @return Returns an integer status code.
             *         0 on success or an error code if the page directory entry was already not present or if the
             *         provided parameters are invalid.
             */
            int freePageDirectoryEntry(const Chunk* chunk, const void* virtualAddr);

            /**
             * @brief Marks as unset multiple page directory entries across a virtual address range.
             *
             * This function removes a series of page directory entries corresponding to a specified virtual address range,
             * freeing all associated page tables.
             *
             * @param chunk A pointer to the chunk structure containing the page directory.
             * @param virtualAddrStart The beginning address of the virtual memory ranges to deallocate.
             * @param virtualAddrEnd The ending address (exclusive) of the virtual memory range to deallocate.         *
             * @return Returns an integer status code.
             *         0 on success or an error code if the provided parameters are invalid or if any page table could
             *         not be properly freed.
             */
            int freePageDirectoryEntries(const Chunk* chunk,
                                         const void* virtualAddrStart, const void* virtualAddrEnd);
        }
    }
}
#endif //PAGING_H
