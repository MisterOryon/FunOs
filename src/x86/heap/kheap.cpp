//
// Created by oryon on 6/1/25.
//

#include "kheap.h"

#include "config.h"
#include "heap.h"
#include "print.h"

funos::heap::Heap gKernelHeap;
funos::heap::HeapTable gKernelHeapTable;

int funos::kheap::initialize()
{
    int res = 0;
    constexpr size_t totalTableEntries = FUNOS_HEAP_SIZE_BYTES / FUNOS_HEAP_BLOCK_SIZE;
    void* start = reinterpret_cast<void*>((FUNOS_HEAP_ADDRESS));
    const void* end = reinterpret_cast<void*>((FUNOS_HEAP_ADDRESS + FUNOS_HEAP_SIZE_BYTES));

    gKernelHeapTable.entries = reinterpret_cast<heap::block_table_entry_t*>((FUNOS_HEAP_TABLE_ADDRESS));
    gKernelHeapTable.total = totalTableEntries;

    res = heap::create(&gKernelHeap, start, end, &gKernelHeapTable);
    if (res < 0) console::writeString("Failed to create kernel heap!\n");
    return res;
}

void* funos::kheap::malloc(const size_t size)
{
    return heap::malloc(&gKernelHeap, size);
}

void funos::kheap::free(const void* ptr)
{
    heap::free(&gKernelHeap, ptr);
}
