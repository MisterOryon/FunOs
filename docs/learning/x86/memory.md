# Memory

## Memory Allocation

There are two types of memory: stack and heap:

- **Stack**: Used for static memory allocation and directly managed by the compiler to allocate local variables.
  Each thread has its own stack.
- **Heap**: A region of memory that is dynamically allocated and freed.
  It's generally larger than the stack and can grow and shrink as needed to accommodate the data size being stored.

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
