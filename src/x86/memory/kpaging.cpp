//
// Created by oryon on 8/1/25.
//

#include "kpaging.h"

#include <status.h>

#include "paging.h"

using namespace funos::memory::mmu;

/**
 * Initializes kernel paging by setting up the necessary memory mappings layout.
 */
int funos::kmemory::initialize(const Chunk* kernelChunk)
{
    int res = 0;

    /**
     * Allocate one-page table (4MB of memory) for the kernel structures.
     */
    res = initializePageDirectoryEntries(
        kernelChunk,
        reinterpret_cast<void*>(0x00000000), reinterpret_cast<void*>(0x00400000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;

    /**
     * Set the first 1M of memory inherit from reel mode in read-only.
     * Only some area can be written:
     * - Kernel Heap Metadata (0x00008000 to 0x0000F000).
     * - Video Memory (0x000B8000 to 0x000C0000).
     */
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x00000000), reinterpret_cast<void*>(0x00000000),
        reinterpret_cast<void*>(0x00100000),
        PAGING_IS_PRESENT
    );
    if (res < 0) return res;
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x00008000), reinterpret_cast<void*>(0x00008000),
        reinterpret_cast<void*>(0x0000F000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x000B8000), reinterpret_cast<void*>(0x000B8000),
        reinterpret_cast<void*>(0x000C0000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;

    /**
     * Configure kernel bin memory:
     * - .text, .asm, .rodata (0x00100000 to 0x00108000) is read-only.
     * - .data, .bss (0x00108000 to 0x00118000) are read-write.
     */
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x00100000), reinterpret_cast<void*>(0x00100000),
        reinterpret_cast<void*>(0x00108000),
        PAGING_IS_PRESENT
    );
    if (res < 0) return res;
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x00108000), reinterpret_cast<void*>(0x00108000),
        reinterpret_cast<void*>(0x00118000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;

    /**
     * Configure kernel stack memory:
     * - Stack Guard (0x00118000 to 0x0011A000).
     * - Stack (0x0011A000 to 0x00400000)
     */
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x00118000), reinterpret_cast<void*>(0x00118000),
        reinterpret_cast<void*>(0x0011A000),
        PAGING_IS_PRESENT
    );
    if (res < 0) return res;
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x0011A000), reinterpret_cast<void*>(0x0011A000),
        reinterpret_cast<void*>(0x00400000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;

    /**
     * Configure kernel heap memory (0x01000000 to 0x07400000).
     * Allocate 25 pages table (100MB of memory) for the kernel heap.
     */
    res = initializePageDirectoryEntries(
        kernelChunk,
        reinterpret_cast<void*>(0x01000000), reinterpret_cast<void*>(0x07400000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;
    res = setPagesMapping(
        kernelChunk,
        reinterpret_cast<void*>(0x01000000), reinterpret_cast<void*>(0x01000000),
        reinterpret_cast<void*>(0x07400000),
        PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
    );
    if (res < 0) return res;

    return FUNOS_ALL_OK;
}
