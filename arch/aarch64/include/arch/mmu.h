#ifndef __MANTICORE_AARCH64_MMU_H
#define __MANTICORE_AARCH64_MMU_H

#include <stdint.h>

/// MMU translation map
typedef struct {
	uint64_t ttbr0;
	uint64_t ttbr1;
} mmu_map_t;

#endif
