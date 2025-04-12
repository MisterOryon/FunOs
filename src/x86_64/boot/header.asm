section .multiboot_header
hearder_start:
    ; magic number (multiboot2)
    dd 0xe85250d6
    ; architecture protected modes i386
    dd 0
    ; header lenght
    dd header_end - hearder_start
    ; checksum
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - hearder_start))

    ; end tag
    dd 0
    dd 0
    dd 8
header_end:
