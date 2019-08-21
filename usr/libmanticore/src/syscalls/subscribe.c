#include <manticore/syscalls.h>

int subscribe(const char *event)
{
	return syscall1(SYS_subscribe, (long) event);
}
