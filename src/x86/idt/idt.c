//
// Created by oryon on 4/22/25.
//

#include "idt.h"

#include "config.h"
#include "memory/memory.h"
#include <terminal/print.h>
#include <io/io.h>

static struct idt_entry g_idt_entries[FUNOS_TOTAL_INTERRUPTS];
static struct idt_pointer g_idt_pointer;

// Function to load the IDT into the CPU (implemented in assembly).
extern void idt_load_table(struct idt_pointer* ptr);

// Default assembly interrupt handler for unhandled interrupts.
extern void idt_no_interrupt(void);

// Assembly interrupt handler for x86 exception.
extern void idt_handle_kernel_panic(void);

// Assembly interrupt handler for keyboard events.
extern void idt_handle_keyboard_interrupt(void);

/**
 * Handles spurious or unused interrupts.
 * This function acts as a default handler for interrupts that don't require specific processing.
 */
void handler_no_interrupt(void)
{
    // Tell the PIC that we have handled the interrupt.
    io_outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Keyboard interrupt handler.
 * This function processes keyboard input by outputs the scancode to the console.
 */
void handle_keyboard_interrupt(void)
{
    // Get keycode from keyboard controller.
    const uint8_t keycode = io_inb(0x60);

    console_write_uint(keycode);
    console_write_string("\n");

    // Tell the PIC that we have handled the interrupt.
    io_outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Kernel panic handler.
 * This function is invoked when a kernel panic occurs upon an exception.
 */
void handle_kernel_panic(void)
{
    console_write_string("Kernel panic!\n");
    while (1);
}

/**
 * Maps an interrupt number to a handler function (address).
 *
 * @param vector The interrupt number to set.
 * @param address The address of the handler function for the interrupt.
 */
static void idt_register_handler(const exception_vector_t vector, void* address)
{
    struct idt_entry* desc = &g_idt_entries[vector];

    // Set the lower 16 bits of the handler address.
    desc->offset_low = (uint32_t)address & IDT_OFFSET_LOW_MASK;

    desc->selector = KERNEL_CODE_SELECTOR;
    desc->reserved = 0x00;

    // Set the type and attributes for the descriptor:
    // - Present (P) bit: 1 (the handler is valid).
    // - Descriptor Privilege Level (DPL): 11 (privilege level 3, allows user-level access).
    // - Gate type: 1110 (interrupt gate).
    desc->type_attr = 0xEE;

    // Set the higher 16 bits of the handler address.
    desc->offset_high = (uint32_t)address >> IDT_OFFSET_HIGH_SHIFT;
}

/**
 * Initializes the Interrupt Descriptor Table (IDT).
 */
void idt_initialize(void)
{
    // Clear all descriptors in the IDT to ensure it starts with a clean state.
    memset(g_idt_entries, 0, sizeof(g_idt_entries));

    // Set the size of the IDT (limit) - 1 because the offset is zero-based.
    g_idt_pointer.limit = sizeof(g_idt_entries) - 1;

    // Set the base address of the IDT table.
    // Cast the descriptor array pointer to uint32_t for proper storage.
    g_idt_pointer.base = (uint32_t)g_idt_entries;

    // Set default interrupt handler.
    for (int i = 0; i < FUNOS_TOTAL_INTERRUPTS; i++)
        idt_register_handler(i, idt_no_interrupt);

    /**
       * Maps standard CPU interrupts to their respective handlers.
       * This sets up handlers for all standard x86 exceptions (interrupts 0-31),
       * which include hardware exceptions, faults, and traps that can occur
       * during normal system operation.
       */
    idt_register_handler(DIVISION_BY_ZERO, idt_handle_kernel_panic);
    idt_register_handler(DEBUG, idt_no_interrupt);
    idt_register_handler(NON_MASKABLE_INTERRUPT, idt_no_interrupt);
    idt_register_handler(BREAKPOINT, idt_no_interrupt);
    idt_register_handler(OVERFLOW, idt_no_interrupt);
    idt_register_handler(BOUND_RANGE_EXCEEDED, idt_handle_kernel_panic);
    idt_register_handler(INVALID_OPCODE, idt_handle_kernel_panic);
    idt_register_handler(DEVICE_NOT_AVAILABLE, idt_handle_kernel_panic);
    idt_register_handler(DOUBLE_FAULT, idt_handle_kernel_panic);
    idt_register_handler(COPROCESSOR_SEGMENT_OVERRUN, idt_handle_kernel_panic);
    idt_register_handler(INVALID_TSS, idt_handle_kernel_panic);
    idt_register_handler(SEGMENT_NOT_PRESENT, idt_handle_kernel_panic);
    idt_register_handler(STACK_SEGMENT_FAULT, idt_handle_kernel_panic);
    idt_register_handler(GENERAL_PROTECTION_FAULT, idt_handle_kernel_panic);
    idt_register_handler(PAGE_FAULT, idt_handle_kernel_panic);
    idt_register_handler(X87_FLOATING_POINT_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(ALIGNMENT_CHECK, idt_handle_kernel_panic);
    idt_register_handler(MACHINE_CHECK, idt_handle_kernel_panic);
    idt_register_handler(SIMD_FLOATING_POINT_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(VIRTUALIZATION_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(CONTROL_PROTECTION_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(HYPERVISOR_INJECTION_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(VMM_COMMUNICATION_EXCEPTION, idt_handle_kernel_panic);
    idt_register_handler(SECURITY_EXCEPTION, idt_handle_kernel_panic);

    /*
     * Maps standard PIC interrupts to their respective handlers.
     */
    idt_register_handler(KEYBOARD_INTERRUPT, idt_handle_keyboard_interrupt);

    // Load the IDT into the processor using the assembly function.
    idt_load_table(&g_idt_pointer);
}
