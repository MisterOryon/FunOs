//
// Created by oryon on 4/22/25.
//

#include "idt.h"

#include "config.h"
#include "kernel.h"
#include "memory/memory.h"

// Array to hold the descriptors for each interrupt.
struct idt_desc idt_descriptors[FUNOS_TOTAL_INTERRUPTS];

// IDTR descriptor that holds the base address and limit of the IDT.
struct idtr_desc idtr_descriptor;

// Function to load the IDT into the CPU (implemented in assembly).
extern void idt_load(struct idtr_desc* ptr);

/**
 * Divide by zero exception handlers.
 * This function is called when a divide-by-zero error occurs.
 */
void idt_zero()
{
    print("Divide by zero \n");
}

/**
 * Maps an interrupt number to a handler function (address).
 *
 * @param interrupt_no The interrupt number to set.
 * @param address The address of the handler function for the interrupt.
 */
void idt_set(const int interrupt_no, void* address)
{
    struct idt_desc* desc = &idt_descriptors[interrupt_no];

    // Set the lower 16 bits of the handler address.
    desc->offset_low = (uint32_t)address & 0x0000FFFF;

    desc->selector = KERNEL_CODE_SELECTOR;
    desc->zero = 0x00;

    // Set the type and attributes for the descriptor:
    // - Present (P) bit: 1 (the handler is valid).
    // - Descriptor Privilege Level (DPL): 11 (privilege level 3, allows user-level access).
    // - Gate type: 1110 (interrupt gate).
    desc->type_attr = 0xEE;

    // Set the higher 16 bits of the handler address.
    desc->offset_high = (uint32_t)address >> 16;
}

/**
 * Initializes the Interrupt Descriptor Table (IDT).
 *
 * This function sets up the IDT by initializing its descriptors to zero,
 * adding custom interrupt handlers (e.g., divide by zero), and loading
 * the IDT into the CPU.
 */
void idt_init()
{
    // Clear all descriptors in the IDT to ensure it starts with a clean state.
    memset(idt_descriptors, 0, sizeof(idt_descriptors));

    // Set the size of the IDT (limit) - 1 because the offset is zero-based.
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;

    // Set the base address of the IDT table.
    // Cast the descriptor array pointer to uint32_t for proper storage.
    idtr_descriptor.base = (uint32_t)idt_descriptors;

    // Map the divide by zero interrupts (interrupt number 0) to its handler.
    // When this interrupt occurs, the idt_zero function is called.
    idt_set(0, idt_zero);

    // Load the IDT into the processor using the assembly function.
    idt_load(&idtr_descriptor);
}
