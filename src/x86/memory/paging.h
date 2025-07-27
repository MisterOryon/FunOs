//
// Created by oryon on 7/26/25.
//

#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

// Page attribute flag bits for x86 paging entries.
#define PAGING_CACHE_DISABLED      0b00010000 // PCD bit: Disables caching for the page
#define PAGING_WRITE_THROUGH       0b00001000 // PWT bit: Enables write-through caching policy
#define PAGING_ACCESS_FROM_ALL     0b00000100 // U/S bit: Allows user-mode access (1) vs supervisor-only (0)
#define PAGING_IS_WRITEABLE        0b00000010 // R/W bit: Allows write access (1) vs read-only (0)
#define PAGING_IS_PRESENT          0b00000001 // P bit: Page is present in memory (1) vs not present (0)

// Constants defining the x86 paging structure dimensions.
#define PAGING_TOTAL_ENTRIES_PER_TABLE     1024
#define PAGING_PAGE_SIZE                   4096

/**
 * Since the lower 12 bits of a page entry are used for flags, this mask isolates the upper 20 bits
 * which contain the actual physical memory address.
 */
#define PAGING_BASE_ADDR_MASK 0xFFFFF000

/**
 * Represents a 4GB paging chunk used for virtual memory management.
 */
struct paging_4gb_chunk
{
    uint32_t* directory_entry; // Pointer to a directory entry structure used in the paging system.
};

/**
 * @brief Initializes and creates a 4GB paging chunk with identity mapping.
 *
 * This function configures an identity mapping (virtual address = physical address) for the entire 4GB
 * address space, with each page table entry and page directory entry set according to the specified flags.
 *
 * @param chunk_4gb A pointer to a struct that will hold the directory entry for the 4GB paging chunk.
 * @param flags Configuration flags to be applied to all page table entries and page directory entry.
 *
 * @return Returns 0 on success (FUNOS_ALL_OK), otherwise returns a negative error code.
 */
int paging_new_4gb(struct paging_4gb_chunk* chunk_4gb, const uint8_t flags);


/**
 * @brief Switches the current paging directory to a new one.
 *
 * This function updates the current paging directory used by the CPU.
 *
 * @param directory A pointer to the page directory to switch to.
 *                  The directory must already be set up with the required paging configurations.
 */
void paging_switch(uint32_t* directory);


/**
 * @brief Enables paging in the processor.
 *
 * This function activates paging by enabling the paging mechanism of the CPU, allowing virtual memory
 * to be mapped to physical memory.
 */
void enable_paging();

/**
 * @brief Retrieves the directory entry of a 4GB paging chunk.
 *
 * This function returns the directory entry pointer associated with the given paging chunk,
 * which represents the starting point for the paging structures.
 *
 * @param chunk A pointer to the structure whose directory entry is stored.
 *
 * @return The directory entry of the specified paging chunk.
 */
uint32_t* paging_4gb_chunk_get_directory(const struct paging_4gb_chunk* chunk);

#endif //PAGING_H
