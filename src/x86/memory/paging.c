//
// Created by oryon on 7/26/25.
//

#include "paging.h"

#include <kheap.h>
#include <status.h>
#include <stdbool.h>

// Function to load a new paging directory into the processor's control register (implemented in assembly).
extern void paging_load_directory(uint32_t* directory);

/**
 * Holds a pointer to the currently active paging directory.
 */
static uint32_t* current_directory = NULL;

/**
 * Calculates the directory and table indexes corresponding to a given virtual address.
 *
 * @param virtual_addr A pointer to the virtual address to be resolved into indexes.
 * @param directory_index_out A pointer to an uint32_t where the directory index will be stored.
 * @param table_index_out A pointer to an uint32_t where the table index will be stored.
 */
static void paging_get_indexes(const void* virtual_addr, uint32_t* directory_index_out, uint32_t* table_index_out)
{
    *directory_index_out =
        ((uint32_t)virtual_addr / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
    *table_index_out =
        (((uint32_t)virtual_addr % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)) / PAGING_PAGE_SIZE);
}

/**
 * Checks if a given address is aligned to the page size.
 */
bool paging_is_aligned(const void* addr)
{
    return ((uint32_t)addr % PAGING_PAGE_SIZE) == 0;
}

/**
 * Switches the current paging directory to a new one.
 */
void paging_switch(uint32_t* directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

/**
 * Initializes a page directory for a memory chunk.
 */
int paging_chunk_initialize_directory(struct chunk* chunk)
{
    int res = FUNOS_ALL_OK;
    uint32_t* directory = NULL;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    directory = kernel_malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    if (directory == NULL)
    {
        res = -ENOMEN;
        goto out;
    }

    // We initialize directory entries as "not present" (undefined).
    // This defers page table allocation until the tables are actually needed, preventing unnecessary
    // memory consumption for unused address spaces.
    for (int pdi = 0; pdi < PAGING_TOTAL_ENTRIES_PER_TABLE; pdi++)
        directory[pdi] = 0x0 | PAGING_LITE_UNSET;

    chunk->directory_entry = directory;

out:
    return res;
}

/**
 * Retrieves the directory associated with a given memory chunk.
 */
uint32_t* paging_chunk_get_directory(const struct chunk* chunk)
{
    if (chunk == NULL)
        return NULL;

    return chunk->directory_entry;
}

/**
 * Completely destroys a chunk's paging directory and all associated page tables.
 */
int paging_chunk_free_memory(const struct chunk* chunk)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    const uint32_t* entry = NULL;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    for (int pdi = 0; pdi < PAGING_TOTAL_ENTRIES_PER_TABLE; pdi++)
    {
        entry = (uint32_t*)&directory[pdi];
        // Only deallocate page tables that were previously allocated.
        // Skip entries marked as "not present" to avoid invalid free operations.
        if (!(*entry & PAGING_LITE_UNSET)) kernel_free((void*)(*entry & PAGING_BASE_ADDR_MASK));
    }
    kernel_free(directory);

out:
    return res;
}

/**
 * Maps a virtual address to a physical address with specific flags.
 */
int paging_set_page_mapping(const struct chunk* chunk,
                            const void* virtual_addr, const void* physical_addr,
                            const uint8_t flags)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    uint32_t* table = NULL;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr) || !paging_is_aligned(physical_addr))
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    paging_get_indexes(virtual_addr, &directory_index, &table_index);

    const uint32_t entry = directory[directory_index];
    // We cannot establish mapping when the page table itself hasn't been allocated.
    // Page tables must be properly initialized before attempting to map virtual addresses.
    if (entry & PAGING_LITE_UNSET)
    {
        res = -EINVAREG;
        goto out;
    }
    table = (uint32_t*)(entry & PAGING_BASE_ADDR_MASK);
    table[table_index] = ((uint32_t)physical_addr) | flags;

out:
    return res;
}

/**
 * Maps a range of virtual addresses to physical addresses with specified flags.
 */
int paging_set_page_mappings(const struct chunk* chunk,
                             const void* virtual_addr_start, const void* physical_addr_start,
                             const void* virtual_addr_end,
                             const uint8_t flags)
{
    int res = FUNOS_ALL_OK;
    uint32_t directory_index_start = 0;
    uint32_t table_index_start = 0;
    uint32_t directory_index_end = 0;
    uint32_t table_index_end = 0;
    uint32_t address_offset = 0;

    if (virtual_addr_start > virtual_addr_end)
    {
        return -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr_start) || !paging_is_aligned(physical_addr_start) ||
        !paging_is_aligned(virtual_addr_end))
    {
        return -EINVAREG;
        goto out;
    }

    paging_get_indexes(virtual_addr_start, &directory_index_start, &table_index_start);
    paging_get_indexes(virtual_addr_end, &directory_index_end, &table_index_end);

    const uint32_t nb_pages = (
        ((directory_index_end - directory_index_start) * PAGING_TOTAL_ENTRIES_PER_TABLE) +
        (table_index_end - table_index_start)
    );
    for (uint32_t i = 0; i < nb_pages; i++)
    {
        res = paging_set_page_mapping(
            chunk,
            (void*)((uint32_t)virtual_addr_start + address_offset),
            (void*)((uint32_t)physical_addr_start + address_offset),
            flags);
        if (res < 0) goto out;
        address_offset += PAGING_PAGE_SIZE;
    }

out:
    return res;
}

/**
 * Translates a virtual address to its physical address and retrieves its page flags.
 */
int paging_resolve_address_mapping(const struct chunk* chunk,
                                   const void* virtual_addr, uint32_t* physical_addr_out,
                                   uint8_t* flags_out)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    const uint32_t* table = NULL;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    if (chunk == NULL || physical_addr_out == NULL || flags_out == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr))
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    paging_get_indexes(virtual_addr, &directory_index, &table_index);

    const uint32_t entry = directory[directory_index];
    if (entry & PAGING_LITE_UNSET)
    {
        res = -EINVAREG;
        goto out;
    }
    table = (uint32_t*)(entry & PAGING_BASE_ADDR_MASK);
    *physical_addr_out = table[table_index] & PAGING_BASE_ADDR_MASK;
    *flags_out = table[table_index] & ~PAGING_BASE_ADDR_MASK;

out:
    return res;
}

/**
 * Configures a page directory entry and allocates its associated page table.
 */
int paging_set_directory_entry(const struct chunk* chunk,
                               const void* virtual_addr,
                               const uint8_t flags)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    uint32_t* entry = NULL;
    void* new_entry = NULL;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr))
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    paging_get_indexes(virtual_addr, &directory_index, &table_index);

    entry = (uint32_t*)&directory[directory_index];
    // If the directory entry is not yet initialized, we allocate a new page table and combine its physical
    // address with the specified flags.
    // Otherwise, preserve the existing page table address and only update the flags.
    if (*entry & PAGING_LITE_UNSET)
    {
        new_entry = kernel_malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        if (new_entry == NULL)
        {
            res = -ENOMEN;
            goto out;
        }
        *entry = (uint32_t)new_entry | flags;
    }
    else
        *entry = (*entry & PAGING_BASE_ADDR_MASK) | flags;

out:
    return
        res;
}

/**
 * Configures multiple page directory entries for a virtual address range.
 */
int paging_set_directory_entries(const struct chunk* chunk,
                                 const void* virtual_addr_start, const void* virtual_addr_end,
                                 const uint8_t flags)
{
    int res = FUNOS_ALL_OK;
    uint32_t directory_index_start = 0;
    uint32_t table_index_start = 0;
    uint32_t directory_index_end = 0;
    uint32_t table_index_end = 0;
    uint32_t address_offset = 0;

    if (virtual_addr_start > virtual_addr_end)
    {
        return -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr_start) || !paging_is_aligned(virtual_addr_end))
    {
        return -EINVAREG;
        goto out;
    }

    paging_get_indexes(virtual_addr_start, &directory_index_start, &table_index_start);
    paging_get_indexes(virtual_addr_end, &directory_index_end, &table_index_end);

    // When the address range ends in the middle of a page table (table_index_end != 0), we must allocate
    // the entire table page.
    // Because page tables can only be allocated as complete units, so partial table allocations are not possible.
    const uint32_t nb_directory_entry = (
        (directory_index_end - directory_index_start) + (table_index_end != 0 ? 1 : 0)
    );
    for (uint32_t i = 0; i < nb_directory_entry; i++)
    {
        res = paging_set_directory_entry(
            chunk,
            (void*)((uint32_t)virtual_addr_start + address_offset),
            flags);
        if (res < 0) goto out;
        address_offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    }

out:
    return res;
}

/**
 * Retrieves the page directory entry for a virtual address with its status and flags.
 */
int paging_get_directory_entry(const struct chunk* chunk,
                               const void* virtual_addr,
                               bool* unset_out, uint8_t* flags_out)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    const uint32_t* entry = NULL;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    if (chunk == NULL || unset_out == NULL || flags_out == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr))
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    paging_get_indexes(virtual_addr, &directory_index, &table_index);

    entry = (uint32_t*)&directory[directory_index];
    *unset_out = (*entry & PAGING_LITE_UNSET);
    *flags_out = (*entry & ~PAGING_BASE_ADDR_MASK);

out:
    return res;
}

/**
 * Removes a page directory entry and frees its associated page table.
 */
int paging_deallocate_directory_entry(const struct chunk* chunk,
                                      const void* virtual_addr)
{
    int res = FUNOS_ALL_OK;
    const uint32_t* directory = NULL;
    uint32_t* entry = NULL;
    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr))
    {
        res = -EINVAREG;
        goto out;
    }

    directory = paging_chunk_get_directory(chunk);
    paging_get_indexes(virtual_addr, &directory_index, &table_index);

    entry = (uint32_t*)&directory[directory_index];
    if (*entry & PAGING_LITE_UNSET)
    {
        res = -EINVAREG;
        goto out;
    }
    kernel_free((void*)(*entry & PAGING_BASE_ADDR_MASK));
    *entry = 0x0 | PAGING_LITE_UNSET;

out:
    return res;
}

/**
 * Deallocates multiple page directory entries across a virtual address range.
 */
int paging_deallocate_directory_entries(const struct chunk* chunk,
                                        const void* virtual_addr_start, const void* virtual_addr_end)
{
    int res = FUNOS_ALL_OK;
    uint32_t directory_index_start = 0;
    uint32_t table_index_start = 0;
    uint32_t directory_index_end = 0;
    uint32_t table_index_end = 0;
    uint32_t address_offset = 0;

    if (chunk == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (virtual_addr_start > virtual_addr_end)
    {
        return -EINVAREG;
        goto out;
    }

    if (!paging_is_aligned(virtual_addr_start) || !paging_is_aligned(virtual_addr_end))
    {
        return -EINVAREG;
        goto out;
    }

    paging_get_indexes(virtual_addr_start, &directory_index_start, &table_index_start);
    paging_get_indexes(virtual_addr_end, &directory_index_end, &table_index_end);

    const uint32_t nb_directory_entry = (directory_index_end - directory_index_start);
    for (uint32_t i = 0; i < nb_directory_entry; i++)
    {
        res = paging_deallocate_directory_entry(
            chunk,
            (void*)((uint32_t)virtual_addr_start + address_offset)
        );
        if (res < 0) goto out;
        address_offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    }

out:
    return res;
}
