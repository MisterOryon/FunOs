//
// Created by oryon on 7/26/25.
//

#include "paging.h"

#include "kheap.h"
#include "status.h"

using namespace funos::memory::mmu;

/**
 * Holds a pointer to the currently active paging directory.
 */
static uint32_t* gCurrentDirectory = nullptr;

/**
 * Calculates the directory and table indexes corresponding to a given virtual address.
 *
 * @param virtualAddr A pointer to the virtual address to be resolved into indexes.
 * @param directoryIndex A pointer to an uint32_t where the page directory entry index will be stored.
 * @param tableIndex A pointer to an uint32_t where the page table entry index will be stored.
 */
static void getPageIndexes(const void* virtualAddr, uint32_t* directoryIndex, uint32_t* tableIndex)
{
    *directoryIndex =
        reinterpret_cast<uint32_t>(virtualAddr) / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    *tableIndex =
        (reinterpret_cast<uint32_t>(virtualAddr) % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE)) /
        PAGING_PAGE_SIZE;
}

/**
 * @brief Retrieves the page directory associated with a given memory chunk.
 *
 * @param chunk The pointer to the chunk whose directory is to be retrieved.
 * @return The pointer to the directory associated with the given chunk, or NULL if the chunk is invalid.
 */
static uint32_t* getPageDirectory(const Chunk* chunk)
{
    if (chunk == nullptr) return nullptr;
    return chunk->pageDirectory;
}

/**
 * @brief Load a new paging directory into the processor's control register.

 * @param directory A pointer to the page directory to be set in the CR3 register.
 */
static void updateCr3(const uint32_t* directory)
{
    asm volatile(
        "mov %%eax, %%cr3\n\t"
        :
        : "a"(directory)
    );
}

void funos::memory::mmu::pagingSwitch(const Chunk* chunk)
{
    uint32_t* pageDirectory = getPageDirectory(chunk);
    updateCr3(pageDirectory);
    gCurrentDirectory = pageDirectory;
}

void funos::memory::mmu::pagingEnable()
{
    asm volatile(
        "mov %%cr0, %%eax\n\t"
        "or $0x80010000, %%eax\n\t"
        "mov %%eax, %%cr0\n\t"
        :
        :
    );
}

bool funos::memory::mmu::isPageAligned(const void* addr)
{
    return (reinterpret_cast<uint32_t>(addr) % PAGING_PAGE_SIZE) == 0;
}

int funos::memory::mmu::initializeChunk(Chunk* chunk)
{
    uint32_t* pageDirectory = nullptr;

    if (chunk == nullptr) return -EINVAREG;

    pageDirectory = static_cast<uint32_t*>(kheap::malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE));
    if (pageDirectory == nullptr) return -ENOMEN;

    // We initialize directory entries as "not present" (undefined).
    // This defers page table allocation until the tables are actually needed, preventing unnecessary
    // memory consumption for unused address spaces.
    for (int pdeIndex = 0; pdeIndex < PAGING_TOTAL_ENTRIES_PER_TABLE; pdeIndex++)
        pageDirectory[pdeIndex] = 0x0 | PAGING_UNSET;
    chunk->pageDirectory = pageDirectory;
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::freeChunk(const Chunk* chunk)
{
    const uint32_t* pageDirectory = nullptr;
    const uint32_t* pde = nullptr;

    if (chunk == nullptr) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    for (int pdeIndex = 0; pdeIndex < PAGING_TOTAL_ENTRIES_PER_TABLE; pdeIndex++)
    {
        pde = const_cast<uint32_t*>(&pageDirectory[pdeIndex]);
        // Only deallocate page tables that were previously allocated.
        // Skip entries marked as "not present" to avoid invalid free operations.
        if (!(*pde & PAGING_UNSET)) kheap::free(reinterpret_cast<void*>(*pde & PAGING_BASE_ADDR_MASK));
    }
    kheap::free(pageDirectory);

    return FUNOS_ALL_OK;
}

int funos::memory::mmu::initializePageDirectoryEntry(const Chunk* chunk, const void* virtualAddr,
                                                     const uint8_t flags)
{
    const uint32_t* pageDirectory = nullptr;
    uint32_t* pde = nullptr;
    uint32_t pdeIndex = 0;
    uint32_t pteIndex = 0;

    if (chunk == nullptr) return -EINVAREG;
    if (!isPageAligned(virtualAddr)) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    getPageIndexes(virtualAddr, &pdeIndex, &pteIndex);

    pde = const_cast<uint32_t*>(&pageDirectory[pdeIndex]);
    // If the page directory entry is not yet initialized, we allocate a new page table and combine its physical
    // address with the specified flags.
    // Otherwise, preserve the existing page table address and only update the flags.
    if (*pde & PAGING_UNSET)
    {
        *pde = reinterpret_cast<uint32_t>(kheap::malloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE));
        if (reinterpret_cast<void*>(*pde) == nullptr) return -ENOMEN;
    }
    *pde = (*pde & PAGING_BASE_ADDR_MASK) | flags;
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::initializePageDirectoryEntries(const Chunk* chunk,
                                                       const void* virtualAddrStart, const void* virtualAddrEnd,
                                                       const uint8_t flags)
{
    int res = FUNOS_ALL_OK;

    uint32_t pdeStart = 0;
    uint32_t pdeEnd = 0;
    uint32_t pteStart = 0;
    uint32_t pteEnd = 0;
    uint32_t addressOffset = 0;

    if (virtualAddrStart > virtualAddrEnd) return -EINVAREG;
    if (!isPageAligned(virtualAddrStart) || !isPageAligned(virtualAddrEnd)) return -EINVAREG;

    getPageIndexes(virtualAddrStart, &pdeStart, &pteStart);
    getPageIndexes(virtualAddrEnd, &pdeEnd, &pteEnd);

    // When the address range ends in the middle of a page table (pteEnd != 0), we must allocate the entire table page.
    // Because page tables can only be allocated as complete units, so partial table allocations are not possible.
    const uint32_t entriesToAllocate = (
        (pdeEnd - pdeStart) + (pteEnd != 0 ? 1 : 0)
    );
    for (uint32_t i = 0; i < entriesToAllocate; i++)
    {
        res = initializePageDirectoryEntry(
            chunk,
            reinterpret_cast<void*>(reinterpret_cast<uint32_t>(virtualAddrStart) + addressOffset),
            flags
        );
        if (res < 0) break;
        addressOffset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    }
    return res;
}

int funos::memory::mmu::setPageMapping(const Chunk* chunk,
                                       const void* virtualAddr, const void* physicalAddr,
                                       const uint8_t flags)
{
    const uint32_t* pageDirectory = nullptr;
    uint32_t pdeIndex = 0;
    uint32_t* pageTable = nullptr;
    uint32_t pteIndex = 0;

    if (chunk == nullptr) return -EINVAREG;
    if (!isPageAligned(virtualAddr) || !isPageAligned(physicalAddr)) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    getPageIndexes(virtualAddr, &pdeIndex, &pteIndex);

    const uint32_t pde = pageDirectory[pdeIndex];
    // We cannot establish mapping when the page table itself hasn't been allocated.
    // Page tables must be properly initialized before attempting to map virtual addresses.
    if (pde & PAGING_UNSET) return -EINVAREG;

    pageTable = reinterpret_cast<uint32_t*>(pde & PAGING_BASE_ADDR_MASK);
    pageTable[pteIndex] = reinterpret_cast<uint32_t>(physicalAddr) | flags;
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::setPagesMapping(const Chunk* chunk,
                                        const void* virtualAddrStart, const void* physicalAddrStart,
                                        const void* virtualAddrEnd,
                                        const uint8_t flags)
{
    int res = FUNOS_ALL_OK;

    uint32_t pdeStartIndex = 0;
    uint32_t pdeEndIndex = 0;
    uint32_t pteStartIndex = 0;
    uint32_t pteEndIndex = 0;
    uint32_t addressOffset = 0;

    if (virtualAddrStart > virtualAddrEnd) return -EINVAREG;
    if (!isPageAligned(virtualAddrStart) || !isPageAligned(physicalAddrStart) || !isPageAligned(virtualAddrEnd))
        return -EINVAREG;

    getPageIndexes(virtualAddrStart, &pdeStartIndex, &pteStartIndex);
    getPageIndexes(virtualAddrEnd, &pdeEndIndex, &pteEndIndex);

    const uint32_t pagesToMap = (
        ((pdeEndIndex - pdeStartIndex) * PAGING_TOTAL_ENTRIES_PER_TABLE) +
        (pteEndIndex - pteStartIndex)
    );
    for (uint32_t i = 0; i < pagesToMap; i++)
    {
        res = setPageMapping(
            chunk,
            reinterpret_cast<void*>(reinterpret_cast<uint32_t>(virtualAddrStart) + addressOffset),
            reinterpret_cast<void*>(reinterpret_cast<uint32_t>(physicalAddrStart) + addressOffset),
            flags);
        if (res < 0) break;
        addressOffset += PAGING_PAGE_SIZE;
    }
    return res;
}

int funos::memory::mmu::getPageMapping(const Chunk* chunk,
                                       const void* virtualAddr, uint32_t* physicalAddr,
                                       uint8_t* flags)
{
    const uint32_t* pageDirectory = nullptr;
    const uint32_t* pte = nullptr;
    uint32_t pdeIndex = 0;
    uint32_t pteIndex = 0;

    if (chunk == nullptr || physicalAddr == nullptr || flags == nullptr) return -EINVAREG;
    if (!isPageAligned(virtualAddr)) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    getPageIndexes(virtualAddr, &pdeIndex, &pteIndex);

    const uint32_t pde = pageDirectory[pdeIndex];
    if (pde & PAGING_UNSET) return -EINVAREG;

    pte = reinterpret_cast<uint32_t*>(pde & PAGING_BASE_ADDR_MASK);
    *physicalAddr = pte[pteIndex] & PAGING_BASE_ADDR_MASK;
    *flags = pte[pteIndex] & ~PAGING_BASE_ADDR_MASK;
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::getPageDirectoryEntryAttributes(const Chunk* chunk,
                                                        const void* virtualAddr,
                                                        bool* isUnset, uint8_t* flags)
{
    const uint32_t* pageDirectory = nullptr;
    const uint32_t* pde = nullptr;
    uint32_t pdeIndex = 0;
    uint32_t pteIndex = 0;

    if (chunk == nullptr || isUnset == nullptr || flags == nullptr) return -EINVAREG;
    if (!isPageAligned(virtualAddr)) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    getPageIndexes(virtualAddr, &pdeIndex, &pteIndex);

    pde = const_cast<uint32_t*>(&pageDirectory[pdeIndex]);
    *isUnset = (*pde & PAGING_UNSET);
    *flags = (*pde & ~PAGING_BASE_ADDR_MASK);
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::freePageDirectoryEntry(const Chunk* chunk, const void* virtualAddr)
{
    const uint32_t* pageDirectory = nullptr;
    uint32_t* pde = nullptr;
    uint32_t pdeIndex = 0;
    uint32_t pteIndex = 0;

    if (chunk == nullptr) return -EINVAREG;
    if (!isPageAligned(virtualAddr)) return -EINVAREG;

    pageDirectory = getPageDirectory(chunk);
    getPageIndexes(virtualAddr, &pdeIndex, &pteIndex);

    pde = const_cast<uint32_t*>(&pageDirectory[pdeIndex]);
    if (*pde & PAGING_UNSET) return -EINVAREG;

    kheap::free(reinterpret_cast<void*>(*pde & PAGING_BASE_ADDR_MASK));
    *pde = 0x0 | PAGING_UNSET;
    return FUNOS_ALL_OK;
}

int funos::memory::mmu::freePageDirectoryEntries(const Chunk* chunk,
                                                 const void* virtualAddrStart, const void* virtualAddrEnd)
{
    int res = FUNOS_ALL_OK;

    uint32_t pdeStartIndex = 0;
    uint32_t pteStartIndex = 0;
    uint32_t pdeEndIndex = 0;
    uint32_t pteEndIndex = 0;
    uint32_t addressOffset = 0;

    if (chunk == nullptr) return -EINVAREG;
    if (virtualAddrStart > virtualAddrEnd) return -EINVAREG;
    if (!isPageAligned(virtualAddrStart) || !isPageAligned(virtualAddrEnd)) return -EINVAREG;

    getPageIndexes(virtualAddrStart, &pdeStartIndex, &pteStartIndex);
    getPageIndexes(virtualAddrEnd, &pdeEndIndex, &pteEndIndex);

    const uint32_t entriesToFree = (pdeEndIndex - pdeStartIndex);
    for (uint32_t i = 0; i < entriesToFree; i++)
    {
        res = freePageDirectoryEntry(
            chunk,
            reinterpret_cast<void*>(reinterpret_cast<uint32_t>(virtualAddrStart) + addressOffset)
        );
        if (res < 0) break;
        addressOffset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE);
    }
    return res;
}
