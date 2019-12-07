#include <manticore/syscalls.h>

int get_io_queue(void **io_queue)
{
	/* FIXME: Execute system call in crt0, and cache the address for this function.  */
	return syscall1(SYS_get_io_queue, (long) io_queue);
}
