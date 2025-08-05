ENTRY(_start)
OUTPUT_FORMAT(binary)
MEMORY
{
    KFLASH (rx) : ORIGIN = 0x00100000, LENGTH = 0x08000
    DRAM   (rw) : ORIGIN = 0x00108000, LENGTH = 0x10000
}
SECTIONS
{
    .text   : ALIGN(4096) { *(.text)   } > KFLASH
    .asm    : ALIGN(4096) { *(.asm)    } > KFLASH
    .rodata : ALIGN(4096) { *(.rodata) } > KFLASH

    .data : ALIGN(4096) { *(.data)          } > DRAM
    .bss  : ALIGN(4096) { *(COMMON) *(.bss) } > DRAM
}
