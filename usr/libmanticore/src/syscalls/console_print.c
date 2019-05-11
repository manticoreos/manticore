#include <manticore/syscalls.h>

void console_print(const char *text)
{
	syscall1(SYS_console_print, (unsigned long) text);
}
