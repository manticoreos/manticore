#include <assert.h>
#include <errno.h>

#include <manticore/syscalls.h>
#include <manticore/vmspace_abi.h>

int main(int argc, char *argv[])
{
	struct vmspace_region vmr = {
		.size = 4096,
	};
	assert(vmspace_alloc(&vmr, sizeof(vmr)) == 0);
	assert(vmr.start != 0);
	assert(vmr.size == 4096);
	// TODO: test you can write to the region

	exit(0);
}
