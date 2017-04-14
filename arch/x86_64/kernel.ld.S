ENTRY(start)
SECTIONS
{
    . = 0x100000;
    .boot : {
        KEEP(*(.multiboot_header))
        *(.boot)
    }
    .text : { *(.text) }
    .rodata : { *(.rodata*) }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
    /DISCARD/ : {
         *(.comment)
         *(.debug*)
    }
    _kernel_end = .;
}
