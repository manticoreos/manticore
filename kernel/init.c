#include <kernel/console.h>

void start_kernel(void)
{
	console_init();
	console_write("Hello, world!\n");
}
