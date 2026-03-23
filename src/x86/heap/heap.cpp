//
// Created by oryon on 6/1/25.
//

#include <stdint.h>

#include "heap.h"
#include "config.h"
#include "memory.h"
#include "status.h"

using namespace funos::heap;

/**
 * Validates the consistency of the heap table by comparing the total blocks calculated from the provided memory
 * range with the total blocks recorded in the heap table.
 *
 * @param ptr The starting pointer of the memory range to be validated.
 * @param end The ending pointer of the memory range to be validated.
 * @param table The heap table containing metadata about the heap's structure.
 * @return 0 if the table is valid, or a negative error code if invalid.
 */
static int validateHeapTable(const void* ptr, const void* end, const HeapTable* table)
{
    int res = FUNOS_ALL_OK;
    const size_t tableSize = reinterpret_cast<uintptr_t>(end) - reinterpret_cast<uintptr_t>(ptr);
    const size_t totalBlocks = tableSize / FUNOS_HEAP_BLOCK_SIZE;

    if (table->total != totalBlocks) res = -EINVAREG;
    return res;
}

/**
 * Validates if a given pointer is aligned to the heap block size.
 *
 * @param ptr The pointer to be validated.
 * @return True if the pointer is aligned to FUNOS_HEAP_BLOCK_SIZE, otherwise false.
 */
static bool isHeapBlockAligned(const void* ptr)
{
    return (reinterpret_cast<uintptr_t>(ptr) % FUNOS_HEAP_BLOCK_SIZE) == 0;
}

/**
 * Aligns a given value to the nearest upper multiple of the heap block size.
 *
 * @param value The size value to be aligned must be a non-negative integer.
 * @return The smallest value greater than or equal to an exact multiple of FUNOS_HEAP_BLOCK_SIZE
 */
static size_t roundUpToBlockSize(size_t value)
{
    if ((value % FUNOS_HEAP_BLOCK_SIZE) == 0) return value;

    value = (value - (value % FUNOS_HEAP_BLOCK_SIZE));
    value += FUNOS_HEAP_BLOCK_SIZE;
    return value;
}

/**
 * Converts a block index within a heap to its corresponding memory address.
 *
 * @param heap Pointer to the heap structure containing the base address and heap metadata.
 * @param block The index of the block within the heap.
 * @return A pointer to the memory address corresponding to the specified block.
 */
static void* getBlockAddress(const Heap* heap, const size_t block)
{
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap->start_addr) + (block * FUNOS_HEAP_BLOCK_SIZE));
}

/**
 * Calculates the block index corresponding to a given memory address within a specified heap.
 *
 * @param heap Pointer to the heap structure containing the relevant heap state.
 * @param address Address within the heap to be converted to a block index.
 * @return The block index within the heap that corresponds to the given address.
 */
static size_t getBlockIndex(const struct Heap* heap, const void* address)
{
    return reinterpret_cast<uintptr_t>(
        (reinterpret_cast<uintptr_t>(address) - reinterpret_cast<uintptr_t>(heap->start_addr)) / FUNOS_HEAP_BLOCK_SIZE
    );
}

/**
 * Finds the starting block index in the heap for a requested number of contiguous free blocks.
 *
 * @param startBlock A pointer to a variable where the starting block will be stored if a suitable range is found.
 * @param heap        A pointer to the heap structure that contains the heap table and other heap-related data.
 * @param totalBlocks The number of contiguous free blocks needed.
 * @return 0 on success with the starting block index set in `start_block`.
 *         or a negative error code if no suitable range of free blocks is available in the heap.
 */
static int getFreeBlockRange(size_t* startBlock, const Heap* heap, const size_t totalBlocks)
{
    int res = FUNOS_ALL_OK;
    const HeapTable* table = heap->table;
    bool foundFreeBlock = false;
    size_t bc = 0;

    for (size_t i = 0; i < table->total; i++)
    {
        if (table->entries[i] & BLOCK_TABLE_ENTRY_FLAG_TAKEN)
        {
            if (foundFreeBlock)
            {
                *startBlock = 0;
                foundFreeBlock = false;
                bc = 0;
            }
            continue;
        }
        if (!foundFreeBlock)
        {
            *startBlock = i;
            foundFreeBlock = true;
        }
        if (++bc == totalBlocks) break;
    }

    if (!foundFreeBlock || bc != totalBlocks) res = -ENOMEN;
    return res;
}

/**
 * Marks a given range of blocks in the heap as taken and updates the heap table accordingly.
 *
 * This function sets the specified blocks in the heap's block table as occupied.
 * It marks the first block with the appropriate flag to indicate it is the start of the allocated section
 * and handles the "has next" flag for blocks that are part of a multi-block allocation.
 *
 * @param heap Pointer to the heap structure where the blocks are located.
 * @param startBlock The starting block index to mark as taken. Must be within the range of the heap.
 * @param totalBlocks The total number of contiguous blocks to mark as taken. Must be greater than zero.
 */
static void markBlocksTaken(const struct Heap* heap, const size_t startBlock, const size_t totalBlocks)
{
    const size_t lastBlock = (startBlock + totalBlocks) - 1;
    block_table_entry_t entry = 0x00;

    entry = BLOCK_TABLE_ENTRY_FLAG_TAKEN | BLOCK_TABLE_ENTRY_FLAG_IS_FIRST;
    if (totalBlocks > 1) entry |= BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;

    for (size_t i = startBlock; i <= lastBlock; i++)
    {
        heap->table->entries[i] = entry;
        entry = 0x00 | BLOCK_TABLE_ENTRY_FLAG_TAKEN;
        if (i + 1 < lastBlock) entry |= BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;
    }
}

/**
 * Allocates a contiguous block of memory for the given number of heap blocks.
 *
 * @param heap A pointer to the heap structure from which memory will be allocated.
 * @param totalBlocks The total number of contiguous blocks to allocate.
 * @return A pointer to the starting address of the allocated memory if successful, or NULL if the allocation fails.
 */
static void* claimMemoryBlocks(const struct Heap* heap, const size_t totalBlocks)
{
    size_t startBlock = 0;

    if (getFreeBlockRange(&startBlock, heap, totalBlocks) < 0) return nullptr;

    markBlocksTaken(heap, startBlock, totalBlocks);
    return getBlockAddress(heap, startBlock);
}

/**
 * Marks a block and all subsequent linked blocks in the heap as free.
 *
 * @param heap Pointer to the heap structure managing the memory.
 * @param startBlock Index of the block to be marked as free, must represent the start of an allocation.
 * @return 0 if the blocks are successfully marked as free,
 *         or a negative error code if the block is invalid or cannot be freed.
 */
static int unclaimMemoryBlocks(const struct Heap* heap, const size_t startBlock)
{
    const HeapTable* table = heap->table;

    if (!(table->entries[startBlock] & (BLOCK_TABLE_ENTRY_FLAG_TAKEN | BLOCK_TABLE_ENTRY_FLAG_IS_FIRST)))
        return -EINVAREG;

    for (size_t i = startBlock; i < table->total; i++)
    {
        block_table_entry_t* entry = &table->entries[i];
        const bool hasNext = *entry & BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;

        *entry = 0x00;
        if (!hasNext) break;
    }
    return FUNOS_ALL_OK;
}

int funos::heap::create(Heap* heap, void* ptr, const void* end, HeapTable* table)
{
    int res = FUNOS_ALL_OK;

    if (heap == nullptr || ptr == nullptr || end == nullptr || ptr >= end || table == nullptr) return -EINVAREG;
    if (!isHeapBlockAligned(ptr) || !isHeapBlockAligned(end)) return -EINVAREG;

    res = validateHeapTable(ptr, end, table);
    if (res < 0) return res;

    memory::memset(heap, 0, sizeof(struct Heap));
    heap->start_addr = ptr;
    heap->table = table;

    const size_t tableSize = sizeof(block_table_entry_t) * table->total;
    memory::memset(table->entries, 0, tableSize);
    return res;
}

void* funos::heap::malloc(const Heap* heap, const size_t size)
{
    void* address = nullptr;

    if (heap == nullptr || size == 0) return nullptr;

    const size_t roundedSize = roundUpToBlockSize(size);
    const size_t totalBlocks = roundedSize / FUNOS_HEAP_BLOCK_SIZE;

    address = claimMemoryBlocks(heap, totalBlocks);
    if (address != nullptr) memory::memset(address, 0, roundedSize);
    return address;
}

int funos::heap::free(const Heap* heap, const void* ptr)
{
    if (heap == nullptr || ptr == nullptr || ptr < heap->start_addr) return -EINVAREG;

    const size_t startBlock = getBlockIndex(heap, ptr);
    if (startBlock >= heap->table->total) return -EINVAREG;
    return unclaimMemoryBlocks(heap, startBlock);
}
