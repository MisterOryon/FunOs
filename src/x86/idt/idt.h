//
// Created by oryon on 4/22/25.
//

#ifndef IDT_H
#define IDT_H
#include <stdint.h>


#define IDT_OFFSET_LOW_MASK 0x0000FFFF
#define IDT_OFFSET_HIGH_SHIFT 16

// The IDT descriptor represents a single entry in the Interrupt Descriptor Table (IDT).
// It defines the address of the interrupt handler and its associated attributes.
struct idt_entry
{
    uint16_t offset_low; // Lower 16 bits of the interrupt handler's address.
    uint16_t selector; // Segment selector for the code segment in the GDT.
    uint8_t reserved; // Must always be zero (reserved for alignment).
    uint8_t type_attr; // Attributes of the interrupt gate (e.g., type, privilege level, present bit).
    uint16_t offset_high; // Higher 16 bits of the interrupt handler's address.
} __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

// The IDTR descriptor is used by the `lidt` instruction to specify the location and size of the IDT.
// It contains the base address of the IDT and its size (in bytes).
struct idt_pointer
{
    uint16_t limit; // Size of the IDT in bytes minus 1.
    uint32_t base; // Base address of the IDT in memory.
} __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

typedef enum : unsigned
{
    DIVISION_BY_ZERO = 0,
    DEBUG = 1,
    NON_MASKABLE_INTERRUPT = 2,
    BREAKPOINT = 3,
    OVERFLOW = 4,
    BOUND_RANGE_EXCEEDED = 5,
    INVALID_OPCODE = 6,
    DEVICE_NOT_AVAILABLE = 7,
    DOUBLE_FAULT = 8,
    COPROCESSOR_SEGMENT_OVERRUN = 9,
    INVALID_TSS = 10,
    SEGMENT_NOT_PRESENT = 11,
    STACK_SEGMENT_FAULT = 12,
    GENERAL_PROTECTION_FAULT = 13,
    PAGE_FAULT = 14,
    // 15 reserved.
    X87_FLOATING_POINT_EXCEPTION = 16,
    ALIGNMENT_CHECK = 17,
    MACHINE_CHECK = 18,
    SIMD_FLOATING_POINT_EXCEPTION = 19,
    VIRTUALIZATION_EXCEPTION = 20,
    CONTROL_PROTECTION_EXCEPTION = 21,
    // 22-27 reserved.
    HYPERVISOR_INJECTION_EXCEPTION = 28,
    VMM_COMMUNICATION_EXCEPTION = 29,
    SECURITY_EXCEPTION = 30,
    // 31 est reserved
} exception_vector_t;

/**
 * @brief Initializes the Interrupt Descriptor Table (IDT).
 */
void idt_initialize(void);

#endif //IDT_H
