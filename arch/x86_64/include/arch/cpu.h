#ifndef X86_CPU_H
#define X86_CPU_H

#include <stdint.h>

static inline uint64_t x86_read_cr2(void)
{
	uint64_t ret;
	asm volatile(
		"mov %%cr2, %0"
		: "=r"(ret));
	return ret;
}

#endif
