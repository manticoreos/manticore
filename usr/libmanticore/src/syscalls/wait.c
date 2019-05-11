#include <manticore/syscalls.h>

void wait()
{
	syscall0(SYS_wait);
}
