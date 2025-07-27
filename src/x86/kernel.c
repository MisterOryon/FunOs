//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include <io.h>
#include <kheap.h>
#include <paging.h>
#include <idt/idt.h>
#include <terminal/print.h>

static struct paging_4gb_chunk* kernel_chunk = 0;

void kernel_main()
{
    display_initialize();

    idt_initialize();
    enable_interrupts();

    if (kernel_heap_init() < 0)
        goto kernel_init_failed;

    kernel_chunk = kernel_malloc(sizeof(struct paging_4gb_chunk));
    if (kernel_chunk == NULL) goto kernel_init_failed;

    if (paging_new_4gb(kernel_chunk,PAGING_IS_WRITEABLE | PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL) < 0)
        goto kernel_init_failed;

    paging_switch(kernel_chunk->directory_entry);
    enable_paging();

    console_write_string("Kernel initialization successful!\n");

    while (1);

kernel_init_failed:
    console_write_string("Kernel initialization failed!\n");
    console_write_string("Need reboot...\n");
    return;
}
