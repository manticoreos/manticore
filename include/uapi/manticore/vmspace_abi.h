#ifndef __MANTICORE_UAPI_VMSPACE_ABI_H
#define __MANTICORE_UAPI_VMSPACE_ABI_H

#include <stdint.h>

// Virtual memory space region.
struct vmspace_region {
	// Size of the virtual memory space region in bytes (input).
	uint64_t	size;
	// Start address of the virtual memory space region (output).
	uint64_t	start;
};

#endif
