#include <assert.h>
#include <errno.h>

#include <manticore/syscalls.h>
#include <manticore/vmspace_abi.h>

int main(int argc, char *argv[])
{
	struct vmspace_region vmr = {
		.size = 4096,
		.align = 4096,
	};
	assert(vmspace_alloc(&vmr, sizeof(vmr)) == 0);
	assert(vmr.start != 0);
	assert(vmr.size == 4096);
	unsigned char *p = (void *) vmr.start;
	for (int i = 0; i < 4096; i++) {
		p[i] = i % 0xff;
	}
	for (int i = 0; i < 4096; i++) {
		assert(p[i] == (i % 0xff));
	}
	exit(0);
}
