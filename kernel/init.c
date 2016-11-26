#include <kernel/interrupts.h>
#include <arch/interrupts.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <stddef.h>

void console_putc(void *unused, char ch)
{
	console_write_char(ch);
}

void start_kernel(void)
{
	console_init();
	init_printf(NULL, console_putc);
	printf("Booting kernel ...\n");
	init_memory_map();
	arch_init_interrupts();
	arch_local_interrupt_enable();
}
