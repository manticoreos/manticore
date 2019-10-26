#include <manticore/syscalls.h>

int getevents(void **events)
{
	/* FIXME: Execute system call in crt0, and cache the address for this function.  */
	return syscall1(SYS_getevents, (long) events);
}
