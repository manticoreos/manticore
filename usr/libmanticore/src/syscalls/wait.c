#include <manticore/syscalls.h>

int wait(void)
{
	return syscall0(SYS_wait);
}
