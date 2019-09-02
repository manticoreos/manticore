#include <manticore/syscalls.h>

ssize_t console_print(const char *text, size_t count)
{
	return syscall2(SYS_console_print, (unsigned long) text, count);
}
