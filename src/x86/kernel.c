//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include <disk.h>
#include <kheap.h>
#include <paging.h>
#include <kpaging.h>
#include <idt/idt.h>
#include <terminal/print.h>

static struct chunk* kernel_chunk = NULL;

void kernel_main()
{
    char* buf;

    display_initialize();

    idt_initialize();
    enable_interrupts();

    if (kernel_heap_init() < 0) goto kernel_init_failed;

    kernel_chunk = kernel_malloc(sizeof(struct chunk));
    if (kernel_chunk == NULL) goto kernel_init_failed;

    if (paging_chunk_initialize_directory(kernel_chunk) < 0) goto kernel_init_failed;
    if (kernel_paging_init(kernel_chunk) < 0) goto kernel_init_failed;
    paging_switch(paging_chunk_get_directory(kernel_chunk));
    enable_paging();

    if (disk_search_and_init() < 0) goto kernel_init_failed;

    console_write_string("Kernel initialization successful!\n");

    buf = kernel_malloc(sizeof(char) * 512);
    if (buf == NULL) goto kernel_init_failed;

    disk_read_block(disk_get(0), 0, 1, buf);

    while (1);

kernel_init_failed:
    console_write_string("Kernel initialization failed!\n");
    console_write_string("Need reboot...\n");
    return;
}
