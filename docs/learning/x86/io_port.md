# IN and OUT in Protected Mode

In x86 architecture, the IN and OUT instructions are used for communication between the processor and devices connected
via I/O ports.
I/O operations can be used in protected mode similarly to real mode.
However, in protected mode, there is the IOPL (I/O Privilege Level).
To perform operations with I/O ports, the CPL (Current Privilege Level) must be greater than or equal to the IOPL.
If an I/O instruction is executed with insufficient privileges, a GP (General Protection Exception) interrupt will
occur.

This means that in protected mode these instructions are often executed in ring 0 (kernel mode).
Applications in user space can perform I/O operations indirectly through system calls that handle and perform I/O
operations for them.

## Assembly Instructions

The IN instruction reads data from an I/O port. The syntax is:

```assembly
IN AL/AX/EAX, DX/port
```

The port number can be directly specified (if it's an 8-bit value) or stored in the DX register.
Data read from the port is stored into:

- AL (for 8-bit data)
- AX (for 16-bit data)
- EAX (for 32-bit data)

Similarly, the OUT instruction sends data to an I/O port. The syntax is:

```assembly
OUT DX/port, AL/AX/EAX
```

The data from AL, AX, or EAX register is written to the specified port.