//
// Created by oryon on 7/26/25.
//

#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Page attribute flag bits for x86 paging entries. */

#define PAGING_CACHE_DISABLED      0b00010000 // PCD bit: Disables caching for the page
#define PAGING_WRITE_THROUGH       0b00001000 // PWT bit: Enables write-through caching policy
#define PAGING_ACCESS_FROM_ALL     0b00000100 // U/S bit: Allows user-mode access (1) vs supervisor-only (0)
#define PAGING_IS_WRITEABLE        0b00000010 // R/W bit: Allows write access (1) vs read-only (0)
#define PAGING_IS_PRESENT          0b00000001 // P bit: Page is present in memory (1) vs not present (0)

/* Custom directory entry flag. */

#define PAGING_LITE_UNSET 0b001000000000 // Unset bit: Indicate that the directory entry is unset.

/* Constants defining the x86 paging structure dimensions. */

#define PAGING_TOTAL_ENTRIES_PER_TABLE     1024
#define PAGING_PAGE_SIZE                   4096

/**
 * Since the lower 12 bits of a page entry are used for flags, this mask isolates the upper 20 bits
 * which contain the actual physical memory address.
 */
#define PAGING_BASE_ADDR_MASK 0xFFFFF000

/**
 * @brief Represents a chunk of the paging mechanism.
 *
 * This structure is part of the paging system and serves as a container for managing directory
 * entries associated with memory paging.
 */
struct chunk
{
    uint32_t* directory_entry; // Pointer to a directory entry structure used in the paging system.
};

/**
 * @brief Checks if a given address is aligned to the page size.
 *
 * This function determines whether the provided address adheres to the alignment requirements
 * of the paging system, which requires addresses to be multiples of the page size.
 *
 * @param addr A pointer to the memory address to check for alignment.
 *
 * @return True if the address is aligned to the page size, otherwise false.
 */
bool paging_is_aligned(const void* addr);

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
 * @brief Initializes a page directory for a memory chunk.
 *
 * This function sets up a page directory for the given chunk in the memory paging system.
 * It allocates and initializes the directory but deliberately marks entries as undefined to optimize memory usage.
 *
 * @note Table pages associated with directory entries are not allocated to conserve memory.
 * Before using paging_set_page_mapping(), you must first configure the directory entry with
 * paging_set_directory_entry(), which will allocate the corresponding table page.
 *
 * @param chunk Pointer to the chunk structure for which the directory is being initialized.
 *
 * @return Returns an integer status code.
 *         0 on success, or an error code if allocation fails or if the input parameter is invalid.
 */
int paging_chunk_initialize_directory(struct chunk* chunk);

/**
 * @brief Retrieves the directory associated with a given memory chunk.
 *
 * This function is used to get the directory pointer for the provided chunk,
 * which is a part of the paging mechanism.
 *
 * @param chunk The pointer to the chunk whose directory is to be retrieved.
 *
 * @return The pointer to the directory associated with the given chunk, or NULL if the chunk is invalid.
 */
uint32_t* paging_chunk_get_directory(const struct chunk* chunk);

/**
 * @brief Completely destroys a chunk's paging directory and all associated page tables.
 *
 * This function performs a comprehensive cleanup of all paging structures owned by a chunk.
 * It systematically iterates through all directory entries, releasing any allocated page tables they reference,
 * and finally frees the memory occupied by the directory itself.
 * This is typically used when a memory context is being destroyed to prevent memory leaks.
 *
 * @param chunk A pointer to the chunk structure whose paging directory should be destroyed.
 *              After this call, the chunk's paging structures will be invalid and should not be used.
 *
 * @return Returns 0 if the paging directory and all its associated resources were successfully freed,
 *         or an error code if:
 *         - The provided parameters are invalid
 */
int paging_chunk_free_memory(const struct chunk* chunk);

/**
 * @brief Maps a virtual address to a physical address with specific page flags.
 *
 * This function establishes a mapping between a virtual memory address and its corresponding physical address
 * in the paging system while applying the specified access control flags.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 *              Must not be NULL.
 * @param virtual_addr The virtual address to be mapped.
 *                     Must be page-aligned.
 * @param physical_addr The physical address to map to.
 *                      Must be page-aligned.
 * @param flags Bit flags controlling page properties (read/write permissions, user/supervisor mode, etc.).
 *
 * @return Returns 0 on successful mapping, or an appropriate error code if:
 *         - Parameters are invalid
 *         - The page directory entry is not properly configured
 */
int paging_set_page_mapping(const struct chunk* chunk,
                            const void* virtual_addr, const void* physical_addr,
                            const uint8_t flags);

/**
 * @brief Maps a range of virtual addresses to physical addresses with specified flags.
 *
 * This function establishes multiple page mappings between a contiguous range of virtual addresses
 * and a corresponding range of physical addresses.
 * It iterates through the specified address range and sets up individual page mappings with consistent flags.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr_start The starting virtual address of the range to map.
 * @param physical_addr_start The starting physical address of the range to map to.
 * @param virtual_addr_end The ending virtual address of the range (exclusive).
 * @param flags Bit flags controlling page properties for all mapped pages in the range
 *              (read/write permissions, user/supervisor mode, etc.).
 *
 * @return Returns 0 on successful mapping of the entire range, or an error code if:
 *         - Any individual page mapping fails
 *         - The address ranges are invalid
 *         - The page directory entry is not properly configured
 */
int paging_set_page_mappings(const struct chunk* chunk,
                             const void* virtual_addr_start, const void* physical_addr_start,
                             const void* virtual_addr_end,
                             const uint8_t flags);

/**
 * @brief Translates a virtual address to its physical address and retrieves its page flags.
 *
 * This function performs virtual-to-physical address translation by traversing the page directory
 * and page tables within the specified chunk.
 * It retrieves both the physical address that corresponds to the provided virtual address
 * and the memory flags associated with the mapping.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr The virtual address to be translated.
 * @param physical_addr_out Pointer where the resolved physical address will be stored.
 * @param flags_out Pointer where the page flags will be stored (permissions, caching attributes, etc.).
 *
 * @return Returns 0 if the translation is successful, or an error code if:
 *         - The page table is not present
 *         - The provided parameters are invalid
 */
int paging_resolve_address_mapping(const struct chunk* chunk,
                                   const void* virtual_addr, uint32_t* physical_addr_out,
                                   uint8_t* flags_out);

/**
 * @brief Sets a directory entry in the paging system.
 *
 * This function updates a directory entry for the specified virtual address in the provided chunk of the paging system.
 * The entry is set with the given flags to define its properties.
 * If required, a new table is allocated for the entry.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr Virtual address associated with the directory entry to be configured.
 * @param flags Additional flags specifying the properties of the entry, such as permissions.
 *
 * @return Returns 0 on success or a negative error code on failure.
 */

/**
 * @brief Configures a page directory entry and allocates its associated page table.
 *
 * This function sets up a directory entry in the paging structure for the specified virtual address.
 * It allocates a new page table if one doesn't already exist for this directory entry, then configures
 * the entry with the specified flags.
 * This is a prerequisite step before mapping individual pages within the corresponding memory region.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr The virtual address whose directory entry needs to be configured.
 *                     The address is used to determine which directory entry to modify.
 * @param flags Control bits that define the properties of the directory entry (present, writable, etc.).
 *
 * @return Returns 0 on successful configuration, or an error code if:
 *         - Memory allocation for the page table fails
 *         - The provided parameters are invalid
 */
int paging_set_directory_entry(const struct chunk* chunk,
                               const void* virtual_addr,
                               const uint8_t flags);

/**
 * @brief Configures multiple page directory entries for a virtual address range.
 *
 * This function prepares the paging structures by setting up all page directory entries that cover
 * the specified virtual address range.
 * For each directory entry in the range, it allocates a page table if one doesn't exist and configures
 * the entry with the provided flags.
 * This establishes the high-level memory mapping structure needed before individual pages can be mapped
 * within these regions.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr_start The starting virtual address of the range to configure.
 *                           Must be aligned to the directory boundary for optimal behavior.
 * @param virtual_addr_end The ending virtual address (exclusive) of the range to configure.
 * @param flags Control bits to apply to all directory entries in the range (present, writable, etc.).
 *
 * @return Returns 0 if all directory entries were successfully configured, or an error code if:
 *         - Memory allocation fails for any page table
 *         - The provided parameters are invalid
 */
int paging_set_directory_entries(const struct chunk* chunk,
                                 const void* virtual_addr_start, const void* virtual_addr_end,
                                 const uint8_t flags);

/**
 * @brief Retrieves the page directory entry for a virtual address with its status and flags.
 *
 * This function examines the page directory to find the entry that controls mapping for the specified
 * virtual address.
 * It provides both the present/absent status of the entry and its configuration flags.
 * This is useful for examining the current state of virtual memory mappings at the directory level before
 * performing additional paging operations.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr The virtual address whose directory entry should be examined.
 *                     The high-order bits of this address determine which directory entry to access.
 * @param unset_out Pointer to a boolean that will be set to true if the directory entry is not present
 *                  or false if it is properly configured.
 * @param flags_out Pointer to a variable where the directory entry's flags will be stored (permissions, etc.).
 *                  The flags are only meaningful if *unset_out is false.
 *
 * @return Returns 0 if the operation completed successfully (regardless of whether the entry exists),
 *         or an error code if:
 *         - The provided parameters are invalid
 */
int paging_get_directory_entry(const struct chunk* chunk,
                               const void* virtual_addr,
                               bool* unset_out, uint8_t* flags_out);

/**
 * @brief Removes a page directory entry and frees its associated page table.
 *
 * This function deallocates memory mapping resources at the directory level for a specified virtual address.
 * It marks the directory entry as not present and releases the memory occupied by the page table that
 * the entry was pointing to.
 * This effectively unmaps an entire region of virtual memory controlled by this directory entry (typically 4MB).
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr The virtual address whose directory entry should be deallocated.
 *                     The high-order bits of this address determine which directory entry to remove.
 *                     All pages mapped within this directory entry's range will become inaccessible.
 *
 * @return Returns 0 if the directory entry was successfully deallocated, or an error code if:
 *         - The directory entry was already not present
 *         - The provided parameters are invalid
 */
int paging_deallocate_directory_entry(const struct chunk* chunk,
                                      const void* virtual_addr);

/**
 * @brief Deallocates multiple page directory entries across a virtual address range.
 *
 * This function removes a series of page directory entries corresponding to a specified virtual address range,
 * freeing all associated page tables.
 * It effectively unmaps large regions of virtual memory at the directory level, releasing both the directory
 * entries themselves and all underlying page table resources they point to.
 *
 * @param chunk A pointer to the chunk structure containing the page directory.
 * @param virtual_addr_start The beginning address of the virtual memory ranges to deallocate.
 *                           This address determines the first directory entry to be processed.
 * @param virtual_addr_end The ending address (exclusive) of the virtual memory range to deallocate.
 *                          This address determines the last directory entry to be processed.
 *
 * @return Returns 0 if all directory entries in the range were successfully deallocated, or an error code if:
 *         - The provided parameters are invalid
 *         - Any page table could not be properly freed
 */
int paging_deallocate_directory_entries(const struct chunk* chunk,
                                        const void* virtual_addr_start, const void* virtual_addr_end);

#endif //PAGING_H
