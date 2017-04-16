#include <arch/vmem-defs.h>

ENTRY(start)
SECTIONS
{
    . = 0x100000;
    .boot : {
        KEEP(*(.multiboot_header))
        *(.boot)
    }
    . += KERNEL_VMA;
    .text : AT(ADDR(.text) - KERNEL_VMA) { *(.text) }
    .rodata : { *(.rodata*) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
    /DISCARD/ : {
         *(.comment)
         *(.debug*)
    }
    _kernel_end = .;
}
