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

## IVT

The IVT (Interrupt Vector Table) is a table residing in memory at address `0x0000` in real mode.
It contains 256 entries, each 4 bytes long (2 bytes for the offset followed by 2 bytes for the segment).
The total size is 1 KB (1024 bytes), and each entry points to an ISR (a piece of code).

The equation to calculate the offset is: `interrupt_num * size_of_entry = offset to edit`
