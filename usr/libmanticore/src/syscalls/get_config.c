#include <manticore/syscalls.h>

int get_config(const char *dev_name, int opt, void *buf, size_t len)
{
	return syscall4(SYS_get_config, (long) dev_name, (long) opt, (long) buf, (long) len);
}
