//
// Created by oryon on 4/22/25.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/**
 * @brief Custom implementation of the `memset` function.
 *
 * This function sets the first `size` bytes of the block in memory, which is pointed to by `ptr`
 * to the specified value `c` (converted to an unsigned char).
 *
 * @param ptr  Pointer to the block of memory to fill.
 * @param c    Value to set each byte to (only the lower 8 bits are used).
 * @param size Number of bytes to set in the memory block.
 * @return     The original pointer, `ptr`.
 */
void* memset(void* ptr, int c, size_t size);

#endif //MEMORY_H
