//
// Created by oryon on 6/1/25.
//

#include "heap.h"
#include <config.h>
#include <memory.h>
#include <status.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Validates the consistency of the heap table by comparing the total blocks calculated from the provided memory
 * range with the total blocks recorded in the heap table.
 *
 * @param ptr The starting pointer of the memory range to be validated.
 * @param end The ending pointer of the memory range to be validated.
 * @param table The heap table containing metadata about the heap's structure.
 * @return 0 if the table is valid, or a negative error code if invalid.
 */
static int heap_validate_table(const void* ptr, const void* end, const struct heap_table* table)
{
    int res = FUNOS_ALL_OK;
    const size_t table_size = (size_t)(end - ptr);
    const size_t total_blocks = table_size / FUNOS_HEAP_BLOCK_SIZE;

    if (table->total != total_blocks)
    {
        res = -EINVAREG;
        goto out;
    }

out:
    return res;
}

/**
 * Validates if a given pointer is aligned to the heap block size.
 *
 * @param ptr The pointer to be validated.
 * @return True if the pointer is aligned to FUNOS_HEAP_BLOCK_SIZE, otherwise false.
 */
static bool heap_validate_alignment(const void* ptr)
{
    return ((uintptr_t)ptr % FUNOS_HEAP_BLOCK_SIZE) == 0;
}

/**
 * Aligns a given value to the nearest upper multiple of the heap block size.
 *
 * @param value The size value to be aligned must be a non-negative integer.
 * @return The smallest value greater than or equal to an exact multiple of FUNOS_HEAP_BLOCK_SIZE
 */
static size_t heap_align_value_to_upper(size_t value)
{
    if ((value % FUNOS_HEAP_BLOCK_SIZE) == 0)
        return value;

    value = (value - (value % FUNOS_HEAP_BLOCK_SIZE));
    value += FUNOS_HEAP_BLOCK_SIZE;
    return value;
}

/**
 * Finds the starting block index in the heap for a requested number of contiguous free blocks.
 *
 * @param start_block A pointer to a variable where the starting block will be stored if a suitable range is found.
 * @param heap        A pointer to the heap structure that contains the heap table and other heap-related data.
 * @param total_blocks The number of contiguous free blocks needed.
 * @return 0 on success with the starting block index set in `start_block`.
 *         or a negative error code if no suitable range of free blocks is available in the heap.
 */
static int heap_get_start_block(size_t* start_block, const struct heap* heap, const size_t total_blocks)
{
    int res = FUNOS_ALL_OK;
    const struct heap_table* table = heap->table;
    bool foundFreeBlock = false;
    size_t bc = 0;

    for (size_t i = 0; i < table->total; i++)
    {
        if (table->entries[i] & HEAP_BLOCK_TABLE_ENTRY_FLAG_TAKEN)
        {
            *start_block = 0;
            foundFreeBlock = false;
            bc = 0;
            continue;
        }
        if (!foundFreeBlock)
        {
            *start_block = i;
            foundFreeBlock = true;
        }
        bc++;
        if (bc == total_blocks)
            break;
    }

    if (!foundFreeBlock || bc != total_blocks)
    {
        res = -ENOMEN;
        goto out;
    }

out:
    return res;
}

/**
 * Converts a block index within a heap to its corresponding memory address.
 *
 * @param heap Pointer to the heap structure containing the base address and heap metadata.
 * @param block The index of the block within the heap.
 * @return A pointer to the memory address corresponding to the specified block.
 */
static void* heap_block_to_address(const struct heap* heap, const size_t block)
{
    return (void*)(heap->start_addr + (block * FUNOS_HEAP_BLOCK_SIZE));
}

/**
 * Marks a given range of blocks in the heap as taken and updates the heap table accordingly.
 *
 * This function sets the specified blocks in the heap's block table as occupied.
 * It marks the first block with the appropriate flag to indicate it is the start of the allocated section
 * and handles the "has next" flag for blocks that are part of a multi-block allocation.
 *
 * @param heap Pointer to the heap structure where the blocks are located.
 * @param start_block The starting block index to mark as taken. Must be within the range of the heap.
 * @param total_blocks The total number of contiguous blocks to mark as taken. Must be greater than zero.
 */
static void heap_mark_blocks_taken(const struct heap* heap, const size_t start_block, const size_t total_blocks)
{
    const size_t end_block = (start_block + total_blocks) - 1;
    heap_block_table_entry_t entry = 0x00;

    entry = HEAP_BLOCK_TABLE_ENTRY_FLAG_TAKEN | HEAP_BLOCK_TABLE_ENTRY_FLAG_IS_FIRST;
    if (total_blocks > 1)
        entry |= HEAP_BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;

    for (size_t i = start_block; i <= end_block; i++)
    {
        heap->table->entries[i] = entry;
        entry = 0x00 | HEAP_BLOCK_TABLE_ENTRY_FLAG_TAKEN;
        if (i + 1 < end_block)
            entry |= HEAP_BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;
    }
}

/**
 * Allocates a contiguous block of memory for the given number of heap blocks.
 *
 * @param heap A pointer to the heap structure from which memory will be allocated.
 * @param total_blocks The total number of contiguous blocks to allocate.
 * @return A pointer to the starting address of the allocated memory if successful, or NULL if the allocation fails.
 */
static void* heap_malloc_blocks(const struct heap* heap, const size_t total_blocks)
{
    void* address = NULL;
    size_t start_block = 0;

    if (heap_get_start_block(&start_block, heap, total_blocks) < 0)
        goto out;

    heap_mark_blocks_taken(heap, start_block, total_blocks);
    address = heap_block_to_address(heap, start_block);

out:
    return address;
}

/**
 * Marks a block and all subsequent linked blocks in the heap as free.
 *
 * @param heap Pointer to the heap structure managing the memory.
 * @param start_block Index of the block to be marked as free, must represent the start of an allocation.
 * @return 0 if the blocks are successfully marked as free,
 *         or a negative error code if the block is invalid or cannot be freed.
 */
static int heap_mark_block_free(const struct heap* heap, const size_t start_block)
{
    int res = FUNOS_ALL_OK;
    const struct heap_table* table = heap->table;

    if (!(table->entries[start_block] & (HEAP_BLOCK_TABLE_ENTRY_FLAG_TAKEN | HEAP_BLOCK_TABLE_ENTRY_FLAG_IS_FIRST)))
    {
        res = -EINVAREG;
        goto out;
    }

    for (size_t i = start_block; i < table->total; i++)
    {
        heap_block_table_entry_t* entry = &table->entries[i];
        const bool hasNext = *entry & HEAP_BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT;

        *entry = 0x00;
        if (!hasNext)
            break;
    }

out:
    return res;
}

/**
 * Calculates the block index corresponding to a given memory address within a specified heap.
 *
 * @param heap Pointer to the heap structure containing the relevant heap state.
 * @param address Address within the heap to be converted to a block index.
 * @return The block index within the heap that corresponds to the given address.
 */
static size_t heap_address_to_block(const struct heap* heap, const void* address)
{
    return ((size_t)(address - heap->start_addr)) / FUNOS_HEAP_BLOCK_SIZE;
}

/**
 * Initializes a heap structure with the given parameters.
 */
int heap_create(struct heap* heap, void* ptr, const void* end, struct heap_table* table)
{
    int res = FUNOS_ALL_OK;

    if (heap == NULL || ptr == NULL || end == NULL || ptr >= end || table == NULL)
    {
        res = -EINVAREG;
        goto out;
    }

    if (!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
    {
        res = -EINVAREG;
        goto out;
    }

    res = heap_validate_table(ptr, end, table);
    if (res < 0)
        goto out;

    memset(heap, 0, sizeof(struct heap));
    heap->start_addr = ptr;
    heap->table = table;

    const size_t table_size = sizeof(heap_block_table_entry_t) * table->total;
    memset(table->entries, 0, table_size);

out:
    return res;
}


/**
 * Allocates a block of memory of the specified size from the heap.
 */
void* heap_malloc(const struct heap* heap, const size_t size)
{
    void* address = NULL;

    if (heap == NULL || size == 0)
        goto out;

    const size_t aligned_size = heap_align_value_to_upper(size);
    const size_t total_blocks = aligned_size / FUNOS_HEAP_BLOCK_SIZE;

    address = heap_malloc_blocks(heap, total_blocks);
    if (address != NULL)
        memset(address, 0, aligned_size);

out:
    return address;
}

/**
 * Frees a previously allocated memory block in the heap.
 */
int heap_free(const struct heap* heap, const void* ptr)
{
    int res = FUNOS_ALL_OK;

    if (heap == NULL || ptr == NULL || ptr < heap->start_addr)
    {
        res = -EINVAREG;
        goto out;
    }

    const size_t start_block = heap_address_to_block(heap, ptr);

    if (start_block >= heap->table->total)
    {
        res = -EINVAREG;
        goto out;
    }
    res = heap_mark_block_free(heap, start_block);

out:
    return res;
}
