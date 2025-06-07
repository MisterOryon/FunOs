//
// Created by oryon on 6/1/25.
//

#include "kheap.h"

#include <config.h>
#include <heap.h>
#include <print.h>

struct heap kernel_heap;
struct heap_table kernel_heap_table;

/**
 * Initializes the kernel heap memory manager.
 */
int kernel_heap_init()
{
    int res = 0;
    const size_t total_table_entries = FUNOS_HEAP_SIZE_BYTES / FUNOS_HEAP_BLOCK_SIZE;
    void* start = (void*)(FUNOS_HEAP_ADDRESS);
    const void* end = (void*)(FUNOS_HEAP_ADDRESS + FUNOS_HEAP_SIZE_BYTES);

    kernel_heap_table.entries = (heap_block_table_entry_t*)(FUNOS_HEAP_TABLE_ADDRESS);
    kernel_heap_table.total = total_table_entries;

    res = heap_create(&kernel_heap, start, end, &kernel_heap_table);
    if (res < 0)
        console_write_string("Failed to create kernel heap!\n");
    return res;
}

/**
 * Allocates a block of memory from the kernel heap.
 */
void* kernel_malloc(const size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

/**
 * Frees previously allocated memory in the kernel heap.
 */
void kernel_free(const void* ptr)
{
    heap_free(&kernel_heap, ptr);
}
