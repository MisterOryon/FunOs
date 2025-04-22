# Interrupts

In computers, an interrupt is a signal sent to the processor when an event occurs and requires immediate attention.
These allow hardware devices and software to communicate with the CPU by interrupting its current activities.

There are 3 kinds of interrupts:

- **Hardware interrupts**: Devices like keyboards, mouse, timers, etc., can trigger interrupts because they need the
  processor to process their input (e.g., a key press on a keyboard).
- **Software interrupts**: Software or applications can trigger interrupts to request a service from the OS, such as
  reading a file, sending data over a network, etc.
- **Exceptions**: These interrupts occur when an error happens, such as division by zero.

When an interrupt is triggered, the processor uses the IVT (Interrupt Vector Table) to determine how to handle it.
Inside the IVT, the processor can locate the appropriate ISR (Interrupt Service Routine) to start a small piece of
code that handles the specific interrupt.

When an interrupt is triggered, the processor saves its current state and jumps to the ISR specified in the IVT.
Once the ISR is completed by executing the IRET (Return from Interrupt) instruction, the processor state is restored,
and execution resumes.

Note that interrupt number `0x80` is commonly used by software interrupts in kernel development, and it is not used by
any BIOS routines.

## Contrasting Real Mode IVT and Protected Mode IDT

The IVT and IDT are two structures utilized for interrupt handling in x86 processors.

- The **IVT** is the initial state of the x86 processor after power-up.
  It is located at the fixed memory range `0000:0000` to `0000:03FF`.
- In contrast, the **IDT** is used when the processor switches to **protected mode** or **long mode**.

Unlike the IVT, the IDT is not fixed at a specific memory location.
The IDT can be positioned anywhere in memory, and the `LIDT` instruction allows the processor to load the IDT register
with the base address and limit of the IDT.

## IVT

The IVT (Interrupt Vector Table) is a table residing in memory at address `0x0000` in real mode.
It contains 256 entries, each 4 bytes long (2 bytes for the offset followed by 2 bytes for the segment).
The total size is 1 KB (1024 bytes), and each entry points to an ISR (a piece of code).

The equation to calculate the offset is: `interrupt_num * size_of_entry = offset to edit`

## IDT

The Interrupt Descriptor Table (IDT) is a data structure used in the x86 architecture to implement an Interrupt Vector
Table (IVT) in protected mode.  
It defines what a processor should do when it receives a specific interrupt.  
The IDT is an array of 256 8-byte (in protected mode) or 16-byte (in long mode) entries, each called a "gate," which
contains the following:

| Bits  | Description                      |
|-------|----------------------------------|
| 0:15  | Offset (Low)                     |
| 16:31 | Segment Selector                 |
| 32:39 | Reserved                         |
| 40:43 | Gate Type                        |
| 44    | Reserved                         |
| 45:46 | Descriptor Privilege Level (DPL) |
| 47    | Present (P)                      |
| 48:63 | Offset (High)                    |

### Entry Details

- **16-bit Segment Selector**  
  Points to the segment containing the handler.
- **32-bit Offset**  
  Points to the handler’s location within the segment.
- **Set of Attributes**:
    - **Gate Type (4 bits)**:
        - `0x5`: Task Gate (offset is not used and should be set to zero).
        - `0x6`: 16-bit Interrupt Gate.
        - `0x7`: 16-bit Trap Gate.
        - `0xE`: 32-bit Interrupt Gate.
        - `0xF`: 32-bit Trap Gate.
    - **DPL (Descriptor Privilege Level)**  
      A 2-bit value determining which CPU privilege levels can access this interrupt using the `INT` instruction.
    - **Present (P)**  
      This bit must be set for the descriptor to be considered valid.

### Types of Gates

There are three types of gates in an IDT:

- **Interrupt Gates**:  
  These are used for hardware interrupts and exceptions.
  When an interrupt gate is issued, the processor disables further interrupts, preventing a hardware interrupt from
  being preempted by another.
- **Trap Gates**:  
  Similar to interrupt gates, but do not disable interrupts, allowing subsequent interrupts to occur.
- **Task Gates**:  
  These are used to switch tasks in a multitasking system.
  A task gate contains a reference to a Task State Segment (TSS), which stores the context (register values, etc.) of a
  task.

### Example Code

```c
typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__ ((packed)) idt_entry_t;

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__ ((packed)) idt_register_t;

idt_entry_t idt_entries[256];
idt_register_t idt_register;
```