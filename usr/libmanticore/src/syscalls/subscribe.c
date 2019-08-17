#include <manticore/syscalls.h>

int subscribe(void)
{
	return syscall0(SYS_subscribe);
}
