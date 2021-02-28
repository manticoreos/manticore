#include <arch/setup.h>

void arch_early_setup(void)
{
	parse_platform_config();
}

void arch_late_setup(void)
{
}
