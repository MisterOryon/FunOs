//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include <io.h>
#include <idt/idt.h>
#include <terminal/print.h>

void kernel_main()
{
    idt_initialize();
    display_initialize();

    while (1);
}
