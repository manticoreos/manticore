#include <manticore/syscalls.h>

int acquire(const char *event, int flags)
{
	return syscall2(SYS_acquire, (long) event, (long) flags);
}
