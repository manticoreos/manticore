#include <kernel/syscall.h>

#include <kernel/errno.h>

int syscall(int nr)
{
	return -ENOSYS;
}
