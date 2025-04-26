//
// Created by oryon on 4/22/25.
//

#include "idt.h"

#include "config.h"
#include "memory/memory.h"
#include <terminal/print.h>

static struct idt_entry g_idt_entries[FUNOS_TOTAL_INTERRUPTS];
static struct idt_pointer g_idt_pointer;

// Function to load the IDT into the CPU (implemented in assembly).
extern void idt_load_table(struct idt_pointer* ptr);

/**
 * Divide by zero exception handlers.
 * This function is called when a divide-by-zero error occurs.
 */
static void idt_handle_division_zero(void)
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
static void idt_handle_debug(void)
{
    console_write_string("Kernel interrupt: debug interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Non-maskable interrupt handler.
 * This function is called when a non-maskable interrupt (NMI) occurs.
 * Kernel execution continues after this interrupt.
 */
static void idt_handle_non_maskable_interrupt(void)
{
    console_write_string("Kernel interrupt: non-maskable interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Breakpoint interrupt handler.
 * This function is called when a breakpoint instruction is executed.
 * Kernel execution continues after this interrupt.
 */
static void idt_handle_breakpoint(void)
{
    console_write_string("Kernel interrupt: breakpoint interrupt occurs.");
    console_write_string("Kernel skip idt");
}

/**
 * Overflow interrupt handler.
 * This function is called when an arithmetic overflow occurs.
 * Kernel execution continues after this interrupt.
 */
static void idt_handle_overflow(void)
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
static void idt_handle_bound_range_exceeded(void)
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
static void idt_handle_invalid_opcode(void)
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
static void idt_handle_device_not_available(void)
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
static void idt_handle_double_fault(void)
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
static void idt_handle_coprocessor_segment_overrun(void)
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
static void idt_handle_invalid_tss(void)
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
static void idt_handle_segment_not_present(void)
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
static void idt_handle_stack_segment_fault(void)
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
static void idt_handle_general_protection_fault(void)
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
static void idt_handle_page_fault(void)
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
static void idt_handle_x87_floating_point_exception(void)
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
static void idt_handle_alignment_check(void)
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
static void idt_handle_machine_check(void)
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
static void idt_handle_simd_floating_point_exception(void)
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
static void idt_handle_virtualization_exception(void)
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
static void idt_handle_control_protection_exception(void)
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
static void idt_handle_hypervisor_injection_exception(void)
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
static void idt_handle_vmm_communication_exception(void)
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
static void idt_handle_security_exception(void)
{
    console_write_string("Kernel interrupt: security exception interrupt occurs.");
    console_write_string("Kernel panic!");
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

    /**
       * Maps standard CPU interrupts to their respective handlers.
       * This sets up handlers for all standard x86 exceptions (interrupts 0-31),
       * which include hardware exceptions, faults, and traps that can occur
       * during normal system operation.
       */
    idt_register_handler(DIVISION_BY_ZERO, idt_handle_division_zero);
    idt_register_handler(DEBUG, idt_handle_debug);
    idt_register_handler(NON_MASKABLE_INTERRUPT, idt_handle_non_maskable_interrupt);
    idt_register_handler(BREAKPOINT, idt_handle_breakpoint);
    idt_register_handler(OVERFLOW, idt_handle_overflow);
    idt_register_handler(BOUND_RANGE_EXCEEDED, idt_handle_bound_range_exceeded);
    idt_register_handler(INVALID_OPCODE, idt_handle_invalid_opcode);
    idt_register_handler(DEVICE_NOT_AVAILABLE, idt_handle_device_not_available);
    idt_register_handler(DOUBLE_FAULT, idt_handle_double_fault);
    idt_register_handler(COPROCESSOR_SEGMENT_OVERRUN, idt_handle_coprocessor_segment_overrun);
    idt_register_handler(INVALID_TSS, idt_handle_invalid_tss);
    idt_register_handler(SEGMENT_NOT_PRESENT, idt_handle_segment_not_present);
    idt_register_handler(STACK_SEGMENT_FAULT, idt_handle_stack_segment_fault);
    idt_register_handler(GENERAL_PROTECTION_FAULT, idt_handle_general_protection_fault);
    idt_register_handler(PAGE_FAULT, idt_handle_page_fault);
    idt_register_handler(X87_FLOATING_POINT_EXCEPTION, idt_handle_x87_floating_point_exception);
    idt_register_handler(ALIGNMENT_CHECK, idt_handle_alignment_check);
    idt_register_handler(MACHINE_CHECK, idt_handle_machine_check);
    idt_register_handler(SIMD_FLOATING_POINT_EXCEPTION, idt_handle_simd_floating_point_exception);
    idt_register_handler(VIRTUALIZATION_EXCEPTION, idt_handle_virtualization_exception);
    idt_register_handler(CONTROL_PROTECTION_EXCEPTION, idt_handle_control_protection_exception);
    idt_register_handler(HYPERVISOR_INJECTION_EXCEPTION, idt_handle_hypervisor_injection_exception);
    idt_register_handler(VMM_COMMUNICATION_EXCEPTION, idt_handle_vmm_communication_exception);
    idt_register_handler(SECURITY_EXCEPTION, idt_handle_security_exception);

    // Load the IDT into the processor using the assembly function.
    idt_load_table(&g_idt_pointer);
}
