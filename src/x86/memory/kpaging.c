//
// Created by oryon on 8/1/25.
//

#include "kpaging.h"
#include <paging.h>

/**
 * Initializes kernel paging by setting up the necessary memory mappings layout.
 */
int kernel_paging_init(const struct chunk* kernel_chunk)
{
    int res = 0;

    /**
     * Allocate one-page table (4MB of memory) for the kernel structures.
     */
    res = paging_set_directory_entries(
        kernel_chunk,
        (void*)0x00000000, (void*)0x00400000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;

    /**
     * Set the first 1M of memory inherit from reel mode in read-only.
     * Only some area can be written:
     * - Kernel Heap Metadata (0x00008000 to 0x0000F000).
     * - Video Memory (0x000B8000 to 0x000C0000).
     */
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x00000000, (void*)0x00000000,
        (void*)0x00100000,
        PAGING_IS_PRESENT
    );
    if (res < 0) goto out;
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x00008000, (void*)0x00008000,
        (void*)0x0000F000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x000B8000, (void*)0x000B8000,
        (void*)0x000C0000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;

    /**
     * Configure kernel bin memory:
     * - .text, .asm, .rodata (0x00100000 to 0x00108000) is read-only.
     * - .data, .bss (0x00108000 to 0x00118000) are read-write.
     */
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x00100000, (void*)0x00100000,
        (void*)0x00108000,
        PAGING_IS_PRESENT
    );
    if (res < 0) goto out;
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x00108000, (void*)0x00108000,
        (void*)0x00118000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;

    /**
     * Configure kernel stack memory:
     * - Stack Guard (0x00118000 to 0x0011A000).
     * - Stack (0x0011A000 to 0x00400000)
     */
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x00118000, (void*)0x00118000,
        (void*)0x0011A000,
        PAGING_IS_PRESENT
    );
    if (res < 0) goto out;
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x0011A000, (void*)0x0011A000,
        (void*)0x00400000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;

    /**
     * Configure kernel heap memory (0x01000000 to 0x07400000).
     * Allocate 25 pages table (100MB of memory) for the kernel heap.
     */
    res = paging_set_directory_entries(
        kernel_chunk,
        (void*)0x01000000, (void*)0x07400000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;
    res = paging_set_page_mappings(
        kernel_chunk,
        (void*)0x01000000, (void*)0x01000000,
        (void*)0x07400000,
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) goto out;

out:
    return res;
}
