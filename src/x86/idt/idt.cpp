//
// Created by oryon on 4/22/25.
//

#include "idt.h"

#include "config.h"
#include "memory.h"
#include "print.h"
#include "io.h"

using namespace funos::irq;

static IdtEntry gIdtEntries[FUNOS_TOTAL_INTERRUPTS];
static IdtPointer gIdtPtr;

// Default assembly interrupt handler for unhandled interrupts.
extern "C" void idt_no_interrupt(void);

// Assembly interrupt handler for x86 exception.
extern "C" void idt_handle_kernel_panic(void);

// Assembly interrupt handler for keyboard events.
extern "C" void idt_handle_keyboard_interrupt(void);

/**
 * Handles spurious or unused interrupts.
 * This function acts as a default handler for interrupts that don't require specific processing.
 */
extern "C" void handler_no_interrupt()
{
    // Tell the PIC that we have handled the interrupt.
    funos::io::outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Keyboard interrupt handler.
 * This function processes keyboard input by outputs the scancode to the console.
 */
extern "C" void handle_keyboard_interrupt()
{
    // Get keycode from keyboard controller.
    const unsigned char keycode = funos::io::inb(0x60);

    funos::console::writeUint(keycode);
    funos::console::writeString("\n");

    // Tell the PIC that we have handled the interrupt.
    funos::io::outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Kernel panic handler.
 * This function is invoked when a kernel panic occurs upon an exception.
 */
extern "C" void handle_kernel_panic()
{
    funos::console::writeString("Kernel panic!\n");
    while (true)
    {
    }
}

/**
 * @brief Loads the Interrupt Descriptor Table (IDT) into the CPU.
 *
 * @param ptr A pointer to the IDT descriptor structure containing the base address and size of the IDT.
 */
static void loadTable(IdtPointer* ptr)
{
    asm volatile(
        "lidt %0\n\t"
        :
        : "m"(*ptr)
    );
}

/**
 * @brief Maps an interrupt number to a handler function (address).
 *
 * @param vector The interrupt number to set.
 * @param address The address of the handler function for the interrupt.
 */
static void registerInterruptHandler(const exception_vector_t vector, void* address)
{
    IdtEntry* desc = &gIdtEntries[vector];

    // Set the lower 16 bits of the handler address.
    desc->offsetLow = reinterpret_cast<uint32_t>(address) & IDT_OFFSET_LOW_MASK;

    desc->selector = KERNEL_CODE_SELECTOR;
    desc->reserved = 0x00;

    // Set the type and attributes for the descriptor:
    // - Present (P) bit: 1 (the handler is valid).
    // - Descriptor Privilege Level (DPL): 11 (privilege level 3, allows user-level access).
    // - Gate type: 1110 (interrupt gate).
    desc->typeAttr = 0xEE;

    // Set the higher 16 bits of the handler address.
    desc->offsetHigh = reinterpret_cast<uint32_t>(address) >> IDT_OFFSET_HIGH_SHIFT;
}

void funos::irq::initialize()
{
    // Clear all descriptors in the IDT to ensure it starts with a clean state.
    memory::memset(gIdtEntries, 0, sizeof(gIdtEntries));
    // Set the size of the IDT (limit) - 1 because the offset is zero-based.
    gIdtPtr.limit = sizeof(gIdtEntries) - 1;
    // Set the base address of the IDT table.
    gIdtPtr.base = reinterpret_cast<uint32_t>(gIdtEntries);

    // Set default interrupt handler.
    for (unsigned i = 0; i < FUNOS_TOTAL_INTERRUPTS; i++)
        registerInterruptHandler(static_cast<exception_vector_t>(i), reinterpret_cast<void*>(idt_no_interrupt));

    /**
       * Maps standard CPU interrupts to their respective handlers.
       * This sets up handlers for all standard x86 exceptions (interrupts 0-31),
       * which include hardware exceptions, faults, and traps that can occur
       * during normal system operation.
       */
    registerInterruptHandler(DIVISION_BY_ZERO, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(DEBUG, reinterpret_cast<void*>(idt_no_interrupt));
    registerInterruptHandler(NON_MASKABLE_INTERRUPT, reinterpret_cast<void*>(idt_no_interrupt));
    registerInterruptHandler(BREAKPOINT, reinterpret_cast<void*>(idt_no_interrupt));
    registerInterruptHandler(OVERFLOW, reinterpret_cast<void*>(idt_no_interrupt));
    registerInterruptHandler(BOUND_RANGE_EXCEEDED, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(INVALID_OPCODE, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(DEVICE_NOT_AVAILABLE, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(DOUBLE_FAULT, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(COPROCESSOR_SEGMENT_OVERRUN, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(INVALID_TSS, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(SEGMENT_NOT_PRESENT, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(STACK_SEGMENT_FAULT, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(GENERAL_PROTECTION_FAULT, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(PAGE_FAULT, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(X87_FLOATING_POINT_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(ALIGNMENT_CHECK, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(MACHINE_CHECK, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(SIMD_FLOATING_POINT_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(VIRTUALIZATION_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(CONTROL_PROTECTION_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(HYPERVISOR_INJECTION_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(VMM_COMMUNICATION_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));
    registerInterruptHandler(SECURITY_EXCEPTION, reinterpret_cast<void*>(idt_handle_kernel_panic));

    /*
     * Maps standard PIC interrupts to their respective handlers.
     */
    registerInterruptHandler(KEYBOARD_INTERRUPT, reinterpret_cast<void*>(idt_handle_keyboard_interrupt));

    // Load the IDT into the processor using the assembly function.
    loadTable(&gIdtPtr);
}

void funos::irq::enable()
{
    asm volatile("sti");
}

void funos::irq::disable()
{
    asm volatile("cli");
}

