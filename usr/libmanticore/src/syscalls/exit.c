#include <manticore/syscalls.h>

void exit(int status)
{
	for (;;) {
		syscall1(SYS_exit, status);
	}
}
