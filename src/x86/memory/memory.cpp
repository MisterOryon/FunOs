//
// Created by oryon on 4/22/25.
//

#include "memory.h"

/**
 * Custom implementation of the `memset` function.
 */
void* funos::memory::memset(void* ptr, const int c, const size_t size)
{
    // Cast the input pointer to an unsigned char pointer (1 byte per char).
    auto* cPtr = static_cast<unsigned char*>(ptr);

    for (size_t i = 0; i < size; i++) cPtr[i] = static_cast<unsigned char>(c);
    return ptr;
}
