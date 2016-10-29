#include <kernel/interrupts.h>
#include <arch/interrupts.h>
#include <kernel/console.h>

void start_kernel(void)
{
	console_init();
	console_write("Hello, world!\n");
	arch_init_interrupts();
	arch_local_interrupt_enable();
}
