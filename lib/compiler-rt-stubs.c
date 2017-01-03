#include <kernel/panic.h>

void __floatundidf(void)
{
	panic("Floating point is not supported");
}

void __floatundisf(void)
{
	panic("Floating point is not supported");
}
