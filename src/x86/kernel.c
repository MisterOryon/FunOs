//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include <io.h>
#include <kheap.h>
#include <idt/idt.h>
#include <terminal/print.h>

void kernel_main()
{
    display_initialize();
    idt_initialize();
    if (kernel_heap_init() < 0)
        goto kernel_init_failed;

    char* str = kernel_malloc(sizeof(char) * 100);
    str[0] = 'a';
    str[1] = 'b';
    str[2] = 'c';
    str[3] = '\n';
    str[4] = '\0';

    char* str2 = kernel_malloc(sizeof(char) * 10000);
    str2[0] = 'd';
    str2[1] = 'e';
    str2[2] = 'f';
    str2[3] = '\n';
    str2[4] = '\0';

    console_write_string(str);
    kernel_free(str);

    char* str3 = kernel_malloc(sizeof(char) * 100);
    str[10] = 'g';
    str[11] = 'h';
    str[12] = 'i';
    str[13] = '\n';
    str[14] = '\0';
    kernel_free(str3);

    console_write_string(str2);
    kernel_free(str2);

    while (1);

kernel_init_failed:
    console_write_string("Kernel initialization failed!\n");
    console_write_string("Need reboot...\n");
    return;
}
