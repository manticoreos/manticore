#include <manticore/syscalls.h>

int vmspace_alloc(struct vmspace_region *vmr, size_t size)
{
	return syscall2(SYS_vmspace_alloc, (long) vmr, (long) size);
}
