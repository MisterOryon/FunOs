# PIC (Programmable Interrupt Controller)

The PIC (Programmable Interrupt Controller) is a crucial hardware part in computers that manages IRQs (Interrupt
Requests).
Its goal is to allow various devices to interrupt the processor without requiring constant polling of device status.
The main function of the PIC is prioritizing and managing the interrupts it receives.
When interrupts is allowed, PIC signals the processor to pause its current task and execute the appropriate ISR (
Interrupt Service Routine).

As computer hardware evolved and required more than 8 interrupt lines, systems began to include two PICs, referred to as
the "master" and "slave" PICs, connected together in a cascading configuration.
This arrangement increased the available interrupt lines from 8 to 15 (with one line used for communication between the
PICs).

In terms of programming, the PIC can be configured to:

- Ignore specific interrupts
- Change interrupt priorities
- Remap interrupt numbers

However, the PIC has been largely superseded in modern systems by the APIC (Advanced Programmable Interrupt Controller)
and newer IO-APIC, which offers better performance and more functionality, especially in multiprocessor systems.

## PIC Communication

The communication with the PIC is achieved through I/O ports:

- **Master PIC**: 0x20 (command port) and 0x21 (data port)
- **Slave PIC**: 0xA0 (command port) and 0xA1 (data port)

Basic commands for PIC setup include:

- **ICW (Initialization Command Word)** sequence: Used for initial configuration of the PIC
- **EOI (End of Interrupt)** command: Signals to the PIC that the interrupt handling is complete, and it can resume
  sending interrupts

## Typical IRQ Assignments in Traditional PC Systems

- **IRQ0**: System timer - Used by the system's internal timer or clock
- **IRQ1**: Keyboard controller - Signaled whenever a key is pressed or released on the keyboard
- **IRQ2**: Cascade to the second PIC - Functions as a link between the two PICs
- **IRQ3**: COM2/COM4 serial ports
- **IRQ4**: COM1/COM3 serial ports
- **IRQ5**: LPT2 parallel port or sound card
- **IRQ6**: Floppy disk controller
- **IRQ7**: LPT1 parallel port (also known as the printer port)
- **IRQ8**: Real-time clock (RTC) on the motherboard
- **IRQ9**: General configuration or software redirection - Can be used for legacy hardware support
- **IRQ10**: General configuration (similar to IRQ9)
- **IRQ11**: General configuration (similar to IRQ9)
- **IRQ12**: Mouse on PS/2 connectors
- **IRQ13**: Math co-processor (also known as the Floating Point Unit or FPU)
- **IRQ14**: Primary ATA channel (IDE) hard disk controller
- **IRQ15**: Secondary ATA channel

## Example of Key Press Event Processing

1) A key press is registered by the keyboard hardware, which generates a signal corresponding to the key pressed and
   sends it to the keyboard controller on the motherboard.

2) The keyboard controller receives the signal, translates it into a scancode, and signals an interrupt request to the
   PIC using the IRQ1 line.

3) The PIC receives and processes the interrupt request on IRQ1. If no higher priority interrupts are being serviced,
   the PIC sends the current keyboard interrupt to the processor via the INTR line.

4) The processor suspends its current operation and starts executing the ISR (Interrupt Service Routine), which is
   defined in the IDT (Interrupt Descriptor Table) and points to the address of the function that handles key presses.

5) The ISR for the keyboard reads the scancode from the keyboard controller's data buffer and translates it into the
   appropriate character. If necessary, it updates the system state for further processing.

6) Once the ISR has processed the keystroke, it signals the End of Interrupt (EOI) to the PIC, which allows the
   processor to resume its previous operation.

7) When the key is released, a similar sequence occurs, allowing the system to recognize that the key is no longer being
   pressed.