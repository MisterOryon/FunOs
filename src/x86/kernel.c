//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include <kheap.h>
#include <paging.h>
#include <kpaging.h>
#include <idt/idt.h>
#include <terminal/print.h>

static struct chunk* kernel_chunk = NULL;

void kernel_main()
{
    display_initialize();

    idt_initialize();
    enable_interrupts();

    if (kernel_heap_init() < 0) goto kernel_init_failed;

    kernel_chunk = kernel_malloc(sizeof(struct chunk));
    if (kernel_chunk == NULL) goto kernel_init_failed;

    if (paging_chunk_initialize_directory(kernel_chunk) < 0) goto kernel_init_failed;
    kernel_paging_init(kernel_chunk);
    paging_switch(paging_chunk_get_directory(kernel_chunk));
    enable_paging();

    console_write_string("Kernel initialization successful!\n");

    while (1);

kernel_init_failed:
    console_write_string("Kernel initialization failed!\n");
    console_write_string("Need reboot...\n");
    return;
}
