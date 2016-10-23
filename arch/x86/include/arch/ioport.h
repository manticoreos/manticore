#ifndef ARCH_IOPORT_H
#define ARCH_IOPORT_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port)
{
	uint8_t v;

	asm volatile (
		"inb %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

static inline void outb(uint8_t v, uint16_t port)
{
	asm volatile (
		"outb %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

#endif
