//
// Created by oryon on 4/22/25.
//

#include "memory.h"

/**
 * Custom implementation of the `memset` function.
 */
void* memset(void* ptr, const int c, const size_t size)
{
    // Cast the input pointer to an unsigned char pointer (1 byte per char).
    unsigned char* c_ptr = ptr;

    for (size_t i = 0; i < size; i++)
        c_ptr[i] = (unsigned char)c;

    return ptr;
}
