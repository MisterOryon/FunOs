//
// Created by oryon on 6/1/25.
//

#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>

namespace funos
{
    namespace kheap
    {
        /**
         * @brief Initializes the kernel heap memory manager.
         *
         * This function sets up the kernel heap by creating the heap structure.
         * If the heap creation fails, a message is written to the console to indicate the error.
         *
         * The kernel heap is used for dynamic memory allocation during the runtime of the operating system.
         *
         * @return 0 on successful initialization, or a negative value if the initialization fails.
         */
        int initialize();

        /**
         * @brief Allocates a block of memory from the kernel heap.
         *
         * This function requests a memory block of a specified size from the kernel heap.
         * The allocated memory block is guaranteed to be aligned to the system's block alignment requirements.
         * If the requested size cannot be satisfied due to insufficient memory or a failed allocation,
         * the function returns a null pointer.
         *
         * The kernel heap must be properly initialized using `kernel_heap_init` before invoking this function.
         * The appropriate error handling should be implemented by the caller to handle cases where memory allocation fails.
         *
         * @param size The size of the memory block to be allocated, in bytes.
         * @return A pointer to the beginning of the allocated memory block, or null if the allocation fails.
         */
        void* malloc(size_t size);

        /**
         * @brief Frees previously allocated memory in the kernel heap.
         *
         * This function releases memory previously allocated from the kernel heap by the `kernel_malloc` function.
         * It ensures that the memory is returned to the kernel heap for future allocations.
         *
         * The function uses the underlying heap management system to handle the deallocation process.
         * Calling this function with a pointer that was not allocated by `kernel_malloc` may cause undefined behavior.
         *
         * @param ptr A pointer to the memory block that needs to be freed.
         *            This pointer should have been returned by a prior successful call to `kernel_malloc`.
         */
        void free(const void* ptr);
    }
}

#endif //KHEAP_H
