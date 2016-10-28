#include <kernel/cpu.h>

void arch_halt_cpu(void)
{
	asm volatile (
		"hlt"
		:
		:
		: "memory");
}
