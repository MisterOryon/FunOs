//
// Created by oryon on 6/1/25.
//

#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

namespace funos
{
    namespace heap
    {
        /* These flags are used to track the state and properties of memory blocks in the heap. */

        // Indicates that the block is currently allocated and not available.
        constexpr uint16_t BLOCK_TABLE_ENTRY_FLAG_TAKEN = 0x01;
        // Marks the first block in a multi-block allocation.
        constexpr uint16_t BLOCK_TABLE_ENTRY_FLAG_IS_FIRST = 0x40;
        // Indicates that this block is part of a multi-block allocation and continues to the next block.
        constexpr uint16_t BLOCK_TABLE_ENTRY_FLAG_HAS_NEXT = 0x80;

        /**
         * Represents a single entry in the block table that tracks metadata for one memory block.
         * Each entry is a byte that can hold multiple flags to describe the block's state.
         */
        typedef unsigned char block_table_entry_t;

        // Represents the metadata table for managing all blocks in the heap.
        struct HeapTable
        {
            block_table_entry_t* entries; // Array of entries, one for each block in the heap.
            size_t total; // Total number of blocks managed by this table.
        };

        // Represents a complete heap memory manager.
        struct Heap
        {
            HeapTable* table; // Pointer to the metadata table that tracks block allocation.
            void* start_addr; // Base address of the heap's memory region.
        };

        /**
         * @brief Initializes a heap structure with the given parameters.
         *
         * This function sets up a heap by validating memory alignment and ensuring the heap
         * table corresponds correctly to the provided memory range.
         * After validation, the heap structure is initialized and all heap table entries are set as free.
         *
         * @param heap Pointer to the heap structure to be initialized.
         * @param ptr Pointer to the start of the memory region used by the heap.
         * @param end Pointer to the end of the memory region used by the heap.
         * @param table Pointer to the heap table structure that manages heap metadata.
         *
         * @return Zero on success, or a negative error code if validation fails.
         */
        int create(Heap* heap, void* ptr, const void* end, HeapTable* table);

        /**
         * @brief Allocates a block of memory of the specified size from the heap.
         *
         * This function allocates memory from the provided heap by determining the number
         * of memory blocks required, aligning the size to fit the heap's block size,
         * and marking the required blocks as taken.
         *
         * The memory is zeroed out upon allocation.
         *
         * @param heap Pointer to the heap structure from which memory will be allocated.
         * @param size The size of the memory to allocate in bytes.
         *
         * @return Pointer to the beginning of the allocated memory block,
         *         or NULL if the allocation fails.
         */
        void* malloc(const Heap* heap, size_t size);

        /**
         * @brief Frees a previously allocated memory block in the heap.
         *
         * This function releases a memory block back to the heap, marking it as free and
         * available for future allocations.
         * The provided pointer must correspond to a valid, previously allocated block within the heap.
         *
         * @param heap Pointer to the heap structure that manages the memory.
         * @param ptr Pointer to the memory block to be freed.
         */
        int free(const Heap* heap, const void* ptr);
    }
}

#endif //HEAP_H
