ENTRY(start)
SECTIONS
{
    . = 0x40000000;
    .boot . : { arch/aarch64/boot.o(.text) }
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}
