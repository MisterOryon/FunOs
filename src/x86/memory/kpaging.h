//
// Created by oryon on 8/1/25.
//

#ifndef KPAGING_H
#define KPAGING_H

#include "paging.h"


namespace funos
{
    namespace kmemory
    {
        /**
         * @brief Initializes kernel paging by setting up the necessary memory mappings layout.
         *
         * The function configures mappings for various parts of the kernel's memory, including memory inherited from real mode,
         * kernel binary code, kernel binary data, kernel stack, kernel heap with appropriate access (e.g., read-only, read-write).
         *
         * @param kernelChunk A pointer to a pre-allocated `chunk` structure used for managing kernel memory.
         * @return Returns 0 on successful completion or returns a negative value if an error occurs during the setup.
         */
        int initialize(const funos::memory::mmu::Chunk* kernelChunk);
    }
}

#endif //KPAGING_H
