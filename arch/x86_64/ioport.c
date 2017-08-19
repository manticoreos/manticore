#include <arch/ioport.h>

uint8_t inb(uint16_t port)
{
	uint8_t v;

	asm volatile (
		"inb %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void outb(uint8_t v, uint16_t port)
{
	asm volatile (
		"outb %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

uint16_t inw(uint16_t port)
{
	uint16_t v;

	asm volatile (
		"inw %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void outw(uint16_t v, uint16_t port)
{
	asm volatile (
		"outw %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

uint32_t inl(uint16_t port)
{
	uint32_t v;

	asm volatile (
		"inl %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void outl(uint32_t v, uint16_t port)
{
	asm volatile (
		"outl %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

