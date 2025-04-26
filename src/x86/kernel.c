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

    // Send a power-on reset sequence to the keyboard controller to ensure that the keyboard is in a known state.
    io_outb(0x60, 0xff);

    char* hello = "Hello, world!\n";

    for (unsigned i = 595; i < 600; i++)
    {
        console_write_string(hello);
        console_write_uint(i);
        console_write_string("\n");
    }

    console_write_string("bye\n");
}
