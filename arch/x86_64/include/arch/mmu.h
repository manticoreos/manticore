#ifndef __MANTICORE_X86_MMU_H
#define __MANTICORE_X86_MMU_H

#include <stdint.h>

/// MMU translation map
typedef struct {
	uint64_t cr3;
} mmu_map_t;

#endif
