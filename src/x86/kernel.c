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

    const char* hello = "Hello, world! ";

    for (unsigned i = 0; i < 5; i++)
    {
        console_write_string(hello);
        console_write_uint(i);
        console_write_string("\n");
    }

    for (int i = -5; i < 10; i++)
    {
        console_write_string(hello);
        console_write_int(i);
        console_write_string("\n");
    }

    console_write_string("bye\n");
}
