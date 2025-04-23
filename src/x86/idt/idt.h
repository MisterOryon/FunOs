//
// Created by oryon on 4/22/25.
//

#ifndef IDT_H
#define IDT_H
#include <stdint.h>

// The IDT descriptor represents a single entry in the Interrupt Descriptor Table (IDT).
// It defines the address of the interrupt handler and its associated attributes.
struct idt_desc
{
    uint16_t offset_low; // Lower 16 bits of the interrupt handler's address.
    uint16_t selector; // Segment selector for the code segment in the GDT.
    uint8_t zero; // Must always be zero (reserved for alignment).
    uint8_t type_attr; // Attributes of the interrupt gate (e.g., type, privilege level, present bit).
    uint16_t offset_high; // Higher 16 bits of the interrupt handler's address.
} __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

// The IDTR descriptor is used by the `lidt` instruction to specify the location and size of the IDT.
// It contains the base address of the IDT and its size (in bytes).
struct idtr_desc
{
    uint16_t limit; // Size of the IDT in bytes minus 1.
    uint32_t base; // Base address of the IDT in memory.
} __attribute__((packed)); // Ensures the structure's layout is not altered by compiler padding.

void idt_init();

#endif //IDT_H
