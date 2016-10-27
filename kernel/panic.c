#include <kernel/panic.h>

#include <kernel/console.h>
#include <kernel/cpu.h>

void panic(char *msg)
{
	console_write("Kernel panic: ");
	console_write(msg);
	console_write("\n");
	for (;;) {
		arch_halt_cpu();
	}
}
