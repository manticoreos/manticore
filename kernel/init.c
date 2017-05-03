#include <kernel/interrupts.h>
#include <kernel/page-alloc.h>
#include <arch/interrupts.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <kernel/kmem.h>
#include <kernel/cpu.h>

#include <stddef.h>

void console_putc(void *unused, char ch)
{
	console_write_char(ch);
}

void start_kernel(void)
{
	arch_init_interrupts();
	console_init();
	init_printf(NULL, console_putc);
	printf("Booting kernel ...\n");
	page_alloc_init();
	init_memory_map();
	kmem_init();
	arch_local_interrupt_enable();
#ifdef HAVE_TEST
	test_page_alloc();
#endif
	printf("Halted.\n");
	arch_halt_cpu();
}
