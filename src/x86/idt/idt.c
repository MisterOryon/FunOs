//
// Created by oryon on 4/22/25.
//

#include "idt.h"

#include "config.h"
#include "memory/memory.h"
#include <terminal/print.h>

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
void idt_division_zero()
{
    console_write_string("Kernel interrupt: divide by zero interrupt occurs.\n");
    console_write_string("Kernel panic!\n");
    while (1);
}

/**
 * Debug interrupt handler.
 * This function is called when a debug interrupt occurs.
 * Kernel execution continues after this interrupt.
 */
void idt_debug()
{
    console_write_string("Kernel interrupt: debug interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Non-maskable interrupt handler.
 * This function is called when a non-maskable interrupt (NMI) occurs.
 * Kernel execution continues after this interrupt.
 */
void idt_non_maskable_interrupt()
{
    console_write_string("Kernel interrupt: non-maskable interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Breakpoint interrupt handler.
 * This function is called when a breakpoint instruction is executed.
 * Kernel execution continues after this interrupt.
 */
void idt_breakpoint()
{
    console_write_string("Kernel interrupt: breakpoint interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Overflow interrupt handler.
 * This function is called when an arithmetic overflow occurs.
 * Kernel execution continues after this interrupt.
 */
void idt_overflow()
{
    console_write_string("Kernel interrupt: overflow interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Bound range exceeded interrupt handler.
 * This function is called when a BOUND instruction detects that an operand
 * exceeds the specified boundaries.
 * Causes a system halt (kernel panic).
 */
void idt_bound_range_exceeded()
{
    console_write_string("Kernel interrupt: bound range interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Invalid opcode interrupt handler.
 * This function is called when the processor attempts to execute an invalid instruction.
 * Causes a system halt (kernel panic).
 */
void idt_invalid_opcode()
{
    console_write_string("Kernel interrupt: invalid opcode interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Device not available interrupt handler.
 * This function is called when a coprocessor or extension is not available.
 * Causes a system halt (kernel panic).
 */
void idt_device_not_available()
{
    console_write_string("Kernel interrupt: device not available interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Double fault interrupt handler.
 * This function is called when an exception occurs during the handling
 * of a previous exception.
 * Causes a system halt (kernel panic).
 */
void idt_double_fault()
{
    console_write_string("Kernel interrupt: double fault interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Coprocessor segment overrun interrupt handler.
 * This function is called when a coprocessor exceeds a segment boundary.
 * Causes a system halt (kernel panic).
 */
void idt_coprocessor_segment_overrun()
{
    console_write_string("Kernel interrupt: coprocessor segment overrun interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Invalid TSS interrupt handler.
 * This function is called when there's an attempt to access an invalid TSS (Task State Segment).
 * Causes a system halt (kernel panic).
 */
void idt_invalid_tss()
{
    console_write_string("Kernel interrupt: invalid tss interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Segment not present interrupt handler.
 * This function is called when there's an attempt to access a segment that is not present.
 * Causes a system halt (kernel panic).
 */
void idt_segment_not_present()
{
    console_write_string("Kernel interrupt: segment not present interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Stack segment fault interrupt handler.
 * This function is called when an operation exceeds the stack segment limits.
 * Causes a system halt (kernel panic).
 */
void idt_stack_segment_fault()
{
    console_write_string("Kernel interrupt: stack segment fault interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * General protection fault interrupt handler.
 * This function is called when a protection violation occurs that is not
 * covered by other exceptions.
 * Causes a system halt (kernel panic).
 */
void idt_general_protection_fault()
{
    console_write_string("Kernel interrupt: general protection fault interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Page fault interrupt handler.
 * This function is called when a memory access error occurs in paged memory.
 * Causes a system halt (kernel panic).
 */
void idt_page_fault()
{
    console_write_string("Kernel interrupt: page fault interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * x87 floating-point exception handler.
 * This function is called when a floating-point error occurs
 * on the x87 coprocessor.
 * Causes a system halt (kernel panic).
 */
void idt_x87_floating_point_exception()
{
    console_write_string("Kernel interrupt: x87 floating point exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Alignment check interrupt handler.
 * This function is called when a memory alignment error occurs.
 * Causes a system halt (kernel panic).
 */
void idt_alignment_check()
{
    console_write_string("Kernel interrupt: alignment check interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Machine check interrupt handler.
 * This function is called when a critical hardware error is detected by the processor.
 * Causes a system halt (kernel panic).
 */
void idt_machine_check()
{
    console_write_string("Kernel interrupt: machine check interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * SIMD floating-point exception handler.
 * This function is called when a floating-point error occurs
 * on the SIMD units (SSE, AVX, etc.).
 * Causes a system halt (kernel panic).
 */
void idt_simd_floating_point_exception()
{
    console_write_string("Kernel interrupt: SIMD floating point exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Virtualization exception handler.
 * This function is called when an error related to virtualization operations occurs.
 * Causes a system halt (kernel panic).
 */
void idt_virtualization_exception()
{
    console_write_string("Kernel interrupt: virtualization exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Control protection exception handler.
 * This function is called when a violation of control protection mechanisms occurs.
 * Causes a system halt (kernel panic).
 */
void idt_control_protection_exception()
{
    console_write_string("Kernel interrupt: control protection exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Hypervisor injection exception handler.
 * This function is called when an error related to interrupt injection
 * by the hypervisor occurs.
 * Causes a system halt (kernel panic).
 */
void idt_hypervisor_injection_exception()
{
    console_write_string("Kernel interrupt: hypervisor injection exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * VMM communication exception handler.
 * This function is called when an error in communication with the
 * virtual machine monitor occurs.
 * Causes a system halt (kernel panic).
 */
void idt_vmm_communication_exception()
{
    console_write_string("Kernel interrupt: vmm communication exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
}

/**
 * Security exception handler.
 * This function is called when a violation of processor security mechanisms occurs.
 * Causes a system halt (kernel panic).
 */
void idt_security_exception()
{
    console_write_string("Kernel interrupt: security exception interrupt occurs.");
    console_write_string("Kernel panic!");
    while (1);
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

    /**
      * Maps standard CPU interrupts to their respective handlers.
      * This sets up handlers for all standard x86 exceptions (interrupts 0-31),
      * which include hardware exceptions, faults, and traps that can occur
      * during normal system operation.
      */
    idt_set(0, idt_division_zero);
    idt_set(1, idt_debug);
    idt_set(2, idt_non_maskable_interrupt);
    idt_set(3, idt_breakpoint);
    idt_set(4, idt_overflow);
    idt_set(5, idt_bound_range_exceeded);
    idt_set(6, idt_invalid_opcode);
    idt_set(7, idt_device_not_available);
    idt_set(8, idt_double_fault);
    idt_set(9, idt_coprocessor_segment_overrun);
    idt_set(10, idt_invalid_tss);
    idt_set(11, idt_segment_not_present);
    idt_set(12, idt_stack_segment_fault);
    idt_set(13, idt_general_protection_fault);
    idt_set(14, idt_page_fault);
    // 15 reserved.
    idt_set(16, idt_x87_floating_point_exception);
    idt_set(17, idt_alignment_check);
    idt_set(18, idt_machine_check);
    idt_set(19, idt_simd_floating_point_exception);
    idt_set(20, idt_virtualization_exception);
    idt_set(21, idt_control_protection_exception);
    // 22-27 reserved.
    idt_set(28, idt_hypervisor_injection_exception);
    idt_set(29, idt_vmm_communication_exception);
    idt_set(30, idt_security_exception);

    // Load the IDT into the processor using the assembly function.
    idt_load(&idtr_descriptor);
}
