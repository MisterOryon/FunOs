# Memory

There are two types of memory:

- **Stack**: Used for static memory allocation and directly managed by the compiler to allocate local variables.
  Each thread has its own stack.
- **Heap**: A region of memory that is dynamically allocated and freed.
  It's generally larger than the stack and can grow and shrink as needed to accommodate the data size being stored.

## Stack

The stack is a region of memory that follows the LIFO (Last In, First Out) principle.
Its key structural components include:

- **Stack Pointer (SP)**: A CPU register that points to the top of the stack.
- **Base Pointer (BP)**: Also called the Frame Pointer (FP), it serves as a reference point for accessing local
  variables and parameters within the current stack frame.
- **Stack Frames**: Individual sections of the stack allocated for each function call.

### Stack Frame Structure

A typical stack frame contains:

1. **Function Parameters**: Values passed to the function
2. **Return Address**: Memory address to return to after function completion
3. **Previous Base Pointer**: Saved BP value from the calling function
4. **Saved Registers**: CPU registers that need to be preserved
5. **Local Variables**: Variables declared within the function
6. **Temporary Storage**: Space for intermediate calculations

### Function Call Mechanism

When a function is called, the following sequence occurs:

1. **Preparing Arguments**:

- Arguments are pushed onto the stack in reverse order (right to left)
- This ensures that the first argument is at the lowest address

2. **Calling the Function**:

- The `call` instruction pushes the return address (the address of the instruction following the call) onto the stack
- Program execution jumps to the function's code

3. **Function Prologue**:

- The current BP is pushed onto the stack
- BP is set to the current SP value, establishing a new reference point
- SP is decremented to allocate space for local variables

4. **Function Execution**:

- The function executes its operations
- Local variables are accessed via negative offsets from BP
- Parameters are accessed via positive offsets from BP

5. **Function Epilogue**:

- SP is restored to the BP value, effectively deallocating local variables
- The previous BP value is popped from the stack and restored
- The `ret` instruction pops the return address and jumps to it

6. **Cleanup**:

- Depending on the calling convention, either the caller or callee clears parameters from the stack

### Stack Memory Management

- The stack grows downward in memory on most architectures (from higher to lower addresses)
- Each thread has its own stack
- Stack size is typically fixed at program startup
- Stack overflow occurs when a program attempts to use more stack space than allocated

This stack mechanism enables nested function calls, recursion, and efficient local variable management in a
deterministic and controlled way.

### Stack Layout Visualization

```text
+---------------------------+
| Parameter c               |  +16
+---------------------------+
| Parameter b               |  +12
+---------------------------+
| Parameter a               |  +8
+---------------------------+
| Return Address            |  +4
+---------------------------+
| Old Base Pointer (BP)     |  +0  <-- BP points here
+---------------------------+
| Saved Register 1          |  -4
+---------------------------+
| Saved Register 2          |  -8
+---------------------------+
| Local Variable 1          |  -12
+---------------------------+
| Local Variable 2          |  -16
+---------------------------+
| ...                       |
+---------------------------+
| Local Variable n          |  -(4*n+8)
+---------------------------+ <-- SP points here during execution


Stack Changes During Nested Function Calls
------------------------------------------

        main()                 foo()                 bar()
    +-------------+        +-------------+        +-------------+
    | main's      |        | foo's       |        | bar's       |
    | local vars  |        | local vars  |        | local vars  |
    +-------------+ <--BP  +-------------+ <--BP  +-------------+ <--BP
    | saved BP    |        | saved BP    |        | saved BP    |
    +-------------+        +-------------+        +-------------+
    | ret addr    |        | ret addr    |        | ret addr    |
    +-------------+        +-------------+        +-------------+
    | ...         |        | args to foo |        | args to bar |
    +-------------+        +-------------+        +-------------+
                           | main's      |        | foo's       |
                           | local vars  |        | local vars  |
                           +-------------+        +-------------+
                           | saved BP    |        | saved BP    |
                           +-------------+        +-------------+
                           | ret addr    |        | ret addr    |
                           +-------------+        +-------------+
                           | ...         |        | args to foo |
                                                  +-------------+
                                                  | main's      |
                                                  | local vars  |
                                                  +-------------+
                                                  | saved BP    |
                                                  +-------------+
                                                  | ret addr    |
                                                  +-------------+
                                                  | ...         |

```

## Heap

In kernel development, we maintain a data structure to track which blocks of memory are currently used
and which are free.
When a program requests a block of memory for allocation, the kernel searches through this data structure to find a free
block that matches the appropriate size and returns a pointer to it after marking it as used.
When a program no longer needs the memory, it can free it, and the kernel will mark the block as free and available for
other allocations.
The heap is typically used when we need to allocate memory at runtime but don't know the size in advance (at compilation
time).
For a program, this allows data structures to grow and shrink as the program runs.
For a kernel, the heap can be used for data shared across different contexts, which is not possible
when using the stack.
