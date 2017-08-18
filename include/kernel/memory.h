#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#define MAX_MEM_REGIONS 8

struct memory_region {
	uint64_t base;
	uint64_t len;
};

extern struct memory_region mem_regions[MAX_MEM_REGIONS];
extern size_t nr_mem_regions;

void init_memory_map(void);
void memory_add_span(uint64_t addr, uint64_t size);

#endif
