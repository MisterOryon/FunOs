//
// Created by oryon on 4/22/25.
//

#include "memory.h"

/**
 * Custom implementation of the `memset` function.
 * Sets the first `size` bytes of the block in memory, which is pointed to by `ptr`
 * to the specified value `c` (converted to an unsigned char).
 *
 * @param ptr  Pointer to the block of memory to fill.
 * @param c    Value to set each byte to (only the lower 8 bits are used).
 * @param size Number of bytes to set in the memory block.
 * @return     The original pointer, `ptr`.
 */
void* memset(void* ptr, const int c, const size_t size)
{
    // Cast the input pointer to an unsigned char pointer (1 byte per char).
    unsigned char* c_ptr = ptr;

    for (size_t i = 0; i < size; i++)
        c_ptr[i] = (unsigned char)c;

    return ptr;
}
