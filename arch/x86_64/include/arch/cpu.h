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

static inline uint64_t rdmsr(uint32_t idx)
{
	uint32_t high, low;
	asm volatile(
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(idx));
	return ((uint64_t) high << 32) | low;
}

static inline void wrmsr(uint32_t idx, uint64_t data)
{
	uint32_t high = data >> 32;
	uint32_t low = data;
	asm volatile(
		"wrmsr"
		:
		: "c"(idx), "a"(low), "d"(high));
}

#endif
