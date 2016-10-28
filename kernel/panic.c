#include <kernel/panic.h>

#include <kernel/console.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>

void panic(char *msg)
{
	console_write("Kernel panic: ");
	console_write(msg);
	console_write("\n");
	arch_local_interrupt_disable();
	for (;;) {
		arch_halt_cpu();
	}
}
