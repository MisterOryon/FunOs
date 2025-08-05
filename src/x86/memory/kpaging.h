//
// Created by oryon on 8/1/25.
//

#ifndef KPAGING_H
#define KPAGING_H

#include <paging.h>


/**
 * @brief Initializes kernel paging by setting up the necessary memory mappings layout.
 *
 * The function configures mappings for various parts of the kernel's memory, including memory inherited from real mode,
 * kernel binary code, kernel binary data, kernel stack, kernel heap with appropriate access (e.g., read-only, read-write).
 *
 * @param kernel_chunk A pointer to a pre-allocated `chunk` structure used for managing kernel memory.
 *
 * @return Returns 0 on successful completion. Returns a negative value if an error occurs during the setup.
 */
int kernel_paging_init(const struct chunk* kernel_chunk);

#endif //KPAGING_H
