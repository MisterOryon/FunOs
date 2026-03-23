//
// Created by oryon on 4/22/25.
//

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

namespace funos
{
    namespace irq
    {
        // IO base address for master PIC.
        constexpr uint8_t PIC1 = 0x20;
        constexpr uint8_t PIC1_COMMAND = PIC1;
        constexpr uint8_t PIC1_DATA = PIC1 + 1;

        // End-of-interrupt command code
        constexpr uint8_t PIC_EOI = 0x20;

        constexpr uint32_t IDT_OFFSET_LOW_MASK = 0x0000FFFF;
        constexpr uint16_t IDT_OFFSET_HIGH_SHIFT = 16;

        /**
         * @brief Initializes the Interrupt Descriptor Table (IDT).
         */
        void initialize();

        /**
         * @brief Enables hardware interrupts.
         */
        void enable();

        /**
         * @brief Disables hardware interrupts.
         */
        void disable();

        // Represents a single entry in the Interrupt Descriptor Table (IDT).
        // It defines the address of the interrupt handler and its associated attributes.
        struct IdtEntry
        {
            uint16_t offsetLow; // Lower 16 bits of the interrupt handler's address.
            uint16_t selector; // Segment selector for the code segment in the GDT.
            uint8_t reserved; // Must always be zero (reserved for alignment).
            uint8_t typeAttr; // Attributes of the interrupt gate (e.g., type, privilege level, present bit).
            uint16_t offsetHigh; // Higher 16 bits of the interrupt handler's address.
        } __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

        // Represents an IDE descriptor of the location and size of the IDT.
        struct IdtPointer
        {
            uint16_t limit; // Size of the IDT in bytes minus 1.
            uint32_t base; // Base address of the IDT in memory.
        } __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

        /* x86-32 bits exceptions */
        typedef enum : unsigned
        {
            // CPU exception: Occurs when dividing by zero or when the result cannot be represented.
            DIVISION_BY_ZERO = 0,
            // CPU exception: Raised when debugging conditions occur (e.g., hardware breakpoints).
            DEBUG = 1,
            // Hardware interrupt: Cannot be disabled, typically signals critical hardware failures.
            NON_MASKABLE_INTERRUPT = 2,
            // CPU exception: Triggered by INT3 instruction, used for software debugging.
            BREAKPOINT = 3,
            // CPU exception: Result of an arithmetic operation is too large (INTO instruction).
            OVERFLOW = 4,
            // CPU exception: BOUND instruction detects an index outside array boundaries.
            BOUND_RANGE_EXCEEDED = 5,
            // CPU exception: Processor encounters an undefined opcode.
            INVALID_OPCODE = 6,
            // CPU exception: Attempt to use FPU/MMX/SSE when not available.
            DEVICE_NOT_AVAILABLE = 7,
            // CPU exception: Error occurs while handling another exception (serious system error).
            DOUBLE_FAULT = 8,
            // CPU exception: Floating-point instruction exceeds segment limit (legacy 387 error).
            COPROCESSOR_SEGMENT_OVERRUN = 9,
            // CPU exception: Invalid Task State Segment during task switch.
            INVALID_TSS = 10,
            // CPU exception: Attempt to access segment marked as not present.
            SEGMENT_NOT_PRESENT = 11,
            // CPU exception: Stack operation exceeds stack segment limit or segment not present.
            STACK_SEGMENT_FAULT = 12,
            // CPU exception: General memory protection violation not covered by other exceptions.
            GENERAL_PROTECTION_FAULT = 13,
            // CPU exception: Memory access violation in paged memory (absent/protection/rights).
            PAGE_FAULT = 14,

            // 15 reserved by Intel

            // CPU exception: Floating-point error from x87 FPU (e.g., invalid operation).
            X87_FLOATING_POINT_EXCEPTION = 16,
            // CPU exception: Unaligned memory access when alignment checking enabled.
            ALIGNMENT_CHECK = 17,
            // CPU exception: CPU detected internal hardware error (may be non-recoverable).
            MACHINE_CHECK = 18,
            // CPU exception: Error in SIMD floating-point operations (SSE/AVX).
            SIMD_FLOATING_POINT_EXCEPTION = 19,
            // CPU exception: Error related to virtualization features (EPT violations).
            VIRTUALIZATION_EXCEPTION = 20,
            // CPU exception: Control-flow enforcement technology (CET) violation.
            CONTROL_PROTECTION_EXCEPTION = 21,

            // 22-27 reserved by Intel.

            // CPU exception: Hypervisor-injected exception in virtualized environment.
            HYPERVISOR_INJECTION_EXCEPTION = 28,
            // CPU exception: Error in VM and VMM communication.
            VMM_COMMUNICATION_EXCEPTION = 29,
            // CPU exception: Security-sensitive event in secure mode operation.
            SECURITY_EXCEPTION = 30,

            // 31 reserved by Intel.

            // IRQ 0: Programmable Interval Timer (PIT) interrupt.
            TIMER_INTERRUPT = 32,
            // IRQ 1: Keyboard controller interrupt.
            KEYBOARD_INTERRUPT = 33,
        } exception_vector_t;
    }
}

#endif //IDT_H
