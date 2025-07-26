# Understanding Paging in 32-Bit Protected Mode

Paging allows physical memory to be divided into fixed-sized "pages" that enable the system to handle larger amounts of
memory, provide support for virtual memory, and ensure process protection and isolation.

The structure of the Page Directory and Page tables is defined by the processor architecture.
Int 32-bits x86 both consist of 1024 32-bit entries.

## Page

A page in 32-bit protected mode is a block of 4096 bytes (4 KiB) of memory.
Pages are described by page tables, similar to how we describe blocks of memory in our heap implement in FunOs.
Virtual addresses are resolved by the processor, which uses translation functions to convert virtual addresses to
physical addresses.
All pages must start at a 4096-byte boundary.

This mechanism enables:

- Mapping virtual addresses to different physical addresses.
  For example, we can map a virtual address block from 0x1000-0x1FFF to a physical address range 0x5000-0x5FFF.
- Creating the impression that all programs are running at the same memory address, even though they are loaded at
  different physical addresses with the same virtual addresses.
- Reconfiguring the virtual address mapping when the processor switches between programs.

### PDE (Page Directory Entry)

Each entry in the Page Directory points to a Page Table, and each entry in a Page Table points to a page in memory.
Moreover, in a 32-bit processor, one-Page Directory can address 4MB of memory through each of its entries, and with 1024
entries, it can address up to 4GB of memory.
The physical address stored in PTE addr is used in combination with the offset from the 12 lower bits to reference a
specific physical memory address of PTE.

```
┌──────────────────────────────────────────────────────────────────────────────────┐ 
│                            32-bit Page Directory Entry                           │ 
├───────────────┬───────┬──────┬───────┬───────┬───────┬───────┬───────┬───────┬───┤ 
│     31 12     │  11-8 │  7   │   6   │   5   │   4   │   3   │   2   │   1   │ 0 │ 
│    Base Addr  │       │  PS  │       │   A   │  PCD  │  PWT  │  U/S  │  R/W  │ P │ 
└───────────────┴───────┴──────┴───────┴───────┴───────┴───────┴───────┴───────┴───┘
```

A Page Directory is defined with the following 32-bit structure:

- **Present (P)**: If this bit is set, the page is currently in physical memory.
  When a page is swapped out, it's not 'Present,' and if a page not in memory is requested, a page fault occurs which
  allows the OS to handle it.
- **Read/Write (R/W)**: Sets the page's permissions.
  If the bit is set, the page has read/write access; if not, it's read-only.
  The WP bit in CR0 determines whether this applies only to userland or both userland and kernel.
- **User/Supervisor (U/S)**: Controls access to the page based on the privilege level.
  If the bit is set, the page can be accessed by everyone; if not, only supervisor can access it.
  To make a page accessible to a user, the user bit must be set in both PDE and PTE.
- **Write-Through (PWT)**: Controls the write-through caching capabilities of the page.
  If it's set, write-through caching is enabled; otherwise, write-back caching is used.
- **Cache Disable (PCD)**: If this bit is set, the page will not be cached; otherwise, it will be cached.
- **Accessed (A)**: Used to determine whether a PDE or PTE was read during the virtual address translation.
  This bit is not cleared by the CPU, so it's the OS's responsibility to do so.
- **The Page Size (PS)**: Stores the size of the page for the given entry.
  If the bit is set, the PDE maps to a 4MiB page if PSE (Page Size Extension) is enabled; if not, it maps to a 4KiB page
  table.
- **Base Addr**: Points to the PTE structure.

### PTE (Page Tables Entry)

```
┌───────────────────────────────────────────────────────────────────────────────────────────┐
│                                 32-bit Page Entry Format                                  │
├───────────────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┬───────┬───┤
│ 31        12  │ 11-9  │   8   │   7   │   6   │   5   │   4   │   3   │   2   │   1   │ 0 │
│   Base Addr   │       │  PAT  │   G   │   D   │   A   │  PCD  │  PWT  │  U/S  │  R/W  │ P │
└───────────────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┴───────┴───┘
```

A page Tables entry in 32-bit protected mode is defined with the following 32-bit structure:

- **Present (P)** : This bit indicates whether the page is currently in physical memory.
  When a page is swapped out, it's marked as not 'Present' in physical memory because when a 'non-present' page is
  accessed, a page fault occurs which allows the OS to handle swapping.
- **Read/Write (R/W)** : This bit sets the page's permissions.
  If the bit is set, the page has read/write access; otherwise, it's read-only.
  The page is always writeable by the kernel, and only when in userland mode can the writeable bit be restricted by CR0.
- **User/Supervisor (U/S)**: This bit controls access to the page based on privilege level.
  If the bit is set, the page can be accessed by everyone; otherwise, only supervisor-level code can access it.
  To make a page accessible to user-level code, the user bit must be set in both the page directory entry and page table
  entry.
- **Page Write Through (PWT)** : This bit controls the write-through caching capabilities of the page.
  If the bit is set, write-through caching is enabled; otherwise, write-back caching is used.
- **Page Cache Disable (PCD)** : If this bit is set, the page will not be cached; otherwise, it will be cached.
- **Accessed (A)** : This bit is used to determine whether a PTE (Page Table Entry) was read during virtual address
  translation.
  This bit is not cleared by the CPU, so it's the OS's responsibility to do so if necessary.
- **Dirty (D)** : This bit is used to identify whether a page has been written to.
  This bit is set by the hardware when the page becomes written to.
- **Global (G)** : This bit instructs the processor to retain the corresponding Translation Lookaside Buffer (TLB) entry
  for the page when a MOV to CR3 instruction is executed.
  To enable global pages, Bit 7 (PGE) in CR4 (control register 4) must be set.
- **Page Attribute Table (PAT)**: If PAT is supported, this bit, along with the PCD (Page Cache Disable) and PWT (Page
  Write Through) bits, will determine the type of memory caching.
  If PAT is not supported, this bit must be set to 0.
- **Base Addr** : Represent the base address of the physical page in memory.
  Because the address is 4KB-aligned, the lower 12 bits are always zero.

## Virtual Address To Physical Address Translation

For a given virtual address V we have:

- The PDI (Page Directory Index): V / 2^22
- The PTI (Page Table Index): (V mod 2^22) / 2^12
- The Offset: V mod 2^12

The physical address stored in base addr in PTE is used in combination with the offset from the 12 lower bits in the
virtual address to reference a specific byte in physical memory.

Note that 2^22 represents the size of the range covered by one page directory entry (4MB) and 2^12 represents the size
of one page (4KB).

For example, address 0x405000 is mapped using the page directory entry at index 1 and the page table entry at index 5:

- PDI = 0x405000 / 2^22 = 0x405000 / 0x400000 = 1
- PTI = (0x405000 mod 0x400000) / 0x1000 = 0x5000 / 0x1000 = 5
- Offset = 0x405000 mod 0x1000 = 0x0

## Enable Paging

To enable paging in 32-bit protected mode, you must set up Page Directory and Page Tables.
Once both are set up, the base address of the Page Directory is loaded into the CR3 control register.
Then, the PG (paging) and PE (protection enable) bits in the CR0 control register must be set to enable paging to
translate all linear addresses into physical addresses using the paging system.
