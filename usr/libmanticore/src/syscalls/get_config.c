#include <manticore/syscalls.h>

int get_config(int desc, int opt, void *buf, size_t len)
{
	return syscall4(SYS_get_config, (long) desc, (long) opt, (long) buf, (long) len);
}
