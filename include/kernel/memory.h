#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stdint.h>

void init_memory_map(void);
void memory_add_span(uint64_t addr, uint64_t size);

#endif
