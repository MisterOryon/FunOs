//
// Created by oryon on 7/26/25.
//

#include "paging.h"

#include <kheap.h>
#include <status.h>

// Function to load a new paging directory into the processor's control register (implemented in assembly).
extern void paging_load_directory(uint32_t* directory);

/**
 * Holds a pointer to the currently active paging directory.
 */
static uint32_t* current_directory = 0;

/*
 * Initializes and creates a 4GB paging chunk with identity mapping.
 */
int paging_new_4gb(struct paging_4gb_chunk* chunk_4gb, const uint8_t flags)
{
    int res = FUNOS_ALL_OK;
    int page_table_offset = 0;

    uint32_t* directory = kernel_malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
    if (directory == NULL)
    {
        res = -ENOMEN;
        goto out;
    }

    for (int pdi = 0; pdi < PAGING_TOTAL_ENTRIES_PER_TABLE; pdi++)
    {
        uint32_t* entry = kernel_malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
        if (entry == NULL)
        {
            for (int i = 0; i < pdi; i++)
                // Apply PAGING_BASE_ADDR_MASK to strip flag bits from the address before freeing memory.
                // This ensures the address passed to free() matches exactly the base address originally
                // returned by kernel_malloc(), since page table entries may contain permission flags
                // in the lower bits (0-11) that aren't part of the actual memory address.
                kernel_free((void*)(directory[i] & PAGING_BASE_ADDR_MASK));

            kernel_free(directory);
            res = -ENOMEN;
            goto out;
        }

        for (int pti = 0; pti < PAGING_TOTAL_ENTRIES_PER_TABLE; pti++)
        {
            // The physical address is calculated as page_table_offset (current 4MB block) + (page index * 4KB).
            // We also include the permission flags (present, writable, etc.) in the lower bits (0-11) of the entry.
            entry[pti] = (page_table_offset + (pti * PAGING_PAGE_SIZE)) | flags;
        }

        // Increment the page table offset to the next 4MB block of physical memory, because each page table
        // controls 4MB (1024 entries × 4KB per page) of addressable memory.
        // This ensures contiguous mapping of the entire 4GB address space across all page tables.
        page_table_offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
        directory[pdi] = (uint32_t)entry | flags;
    }

    chunk_4gb->directory_entry = directory;

out:
    return res;
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
 * Retrieves the directory entry of a 4GB paging chunk.
 */
uint32_t* paging_4gb_chunk_get_directory(const struct paging_4gb_chunk* chunk)
{
    return chunk->directory_entry;
}
