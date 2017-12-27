#include <arch/setup.h>

void arch_early_setup(void)
{
	init_memory_map();
}

void arch_late_setup(void)
{
}
