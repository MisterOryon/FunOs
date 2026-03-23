//
// Created by oryon on 4/26/25.
//

#include "io.h"

using namespace funos;

__attribute__((optimize("-O2"))) unsigned char io::inb(unsigned short port)
{
    unsigned char result;
    asm volatile (
        "xor %%eax, %%eax\n\t"
        "inb %%dx, %%al\n\t"
        : "=a" (result)
        : "d" (port)
    );
    return result;
}

__attribute__((optimize("-O2"))) unsigned short io::inw(unsigned short port)
{
    unsigned short res;
    asm volatile (
        "xor %%eax, %%eax\n\t"
        "in %%dx, %%ax\n\t"
        : "=a"(res)
        : "d"(port));
    return res;
}

__attribute__((optimize("-O2"))) void io::outb(unsigned short port, unsigned char data)
{
    asm volatile (
        "out %%al, %%dx\n\t"
        :
        : "a"(data), "d"(port));
}

__attribute__((optimize("-O2"))) void io::outw(unsigned short port, unsigned short data)
{
    asm volatile (
        "out %%ax, %%dx\n\t"
        :
        : "a"(data), "d"(port));
}
