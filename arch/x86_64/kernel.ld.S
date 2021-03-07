#include <arch/vmem-defs.h>

ENTRY(start)
SECTIONS
{
    . = 0x8000;
    .boot : {
        KEEP(*(.multiboot_header))
        *(.boot)
    }
    . = 0x100000;
    . += KERNEL_VMA;
    .text : AT(ADDR(.text) - KERNEL_VMA) { *(.text) }
    .rodata : { *(.rodata*) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
    /DISCARD/ : {
         *(.comment)
    }
    _kernel_end = .;
}
