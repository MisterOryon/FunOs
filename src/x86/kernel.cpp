//
// Created by oryon on 4/22/25.
//
#include "kernel.h"

#include "disk.h"
#include "kheap.h"
#include "paging.h"
#include "kpaging.h"
#include "idt.h"
#include "print.h"

using namespace funos;

static memory::mmu::Chunk* gKernelChunk = nullptr;

void kernelInitFailed()
{
    console::writeString("Kernel initialization failed!\n");
    console::writeString("Need reboot...\n");
    while (true) asm volatile("hlt");
}

void kernel_main()
{
    char* buf;

    console::initialize();

    irq::initialize();
    irq::enable();

    if (kheap::initialize() < 0) kernelInitFailed();

    gKernelChunk = static_cast<memory::mmu::Chunk*>(kheap::malloc(sizeof(memory::mmu::Chunk)));
    if (gKernelChunk == nullptr) kernelInitFailed();

    if (memory::mmu::initializeChunk(gKernelChunk) < 0) kernelInitFailed();
    if (kmemory::initialize(gKernelChunk) < 0) kernelInitFailed();
    memory::mmu::pagingSwitch(gKernelChunk);
    memory::mmu::pagingEnable();

    if (disk::searchAndInitialize() < 0) kernelInitFailed();

    buf = static_cast<char*>(kheap::malloc(sizeof(char) * 512));
    if (buf == nullptr) kernelInitFailed();

    disk::readBlock(disk::getDisk(0), 0, 1, buf);

    console::writeString("Kernel initialization successful!\n");
    kheap::free(buf);
    while (true) asm volatile("hlt");
}

