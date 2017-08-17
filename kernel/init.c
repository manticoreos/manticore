#include <kernel/page-alloc.h>
#include <kernel/console.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <kernel/kmem.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>
#include <arch/setup.h>

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
	page_alloc_init();
	arch_setup();
	kmem_init();
	arch_local_interrupt_enable();
#ifdef HAVE_TEST
	test_kmem();
	test_page_alloc();
#endif
	printf("Halted.\n");
	arch_halt_cpu();
}
