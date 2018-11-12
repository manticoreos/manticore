#include <arch/ioport.h>

uint8_t pio_read8(uint16_t port)
{
	uint8_t v;

	asm volatile (
		"inb %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void pio_write8(uint8_t v, uint16_t port)
{
	asm volatile (
		"outb %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

uint16_t pio_read16(uint16_t port)
{
	uint16_t v;

	asm volatile (
		"inw %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void pio_write16(uint16_t v, uint16_t port)
{
	asm volatile (
		"outw %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

uint32_t pio_read32(uint16_t port)
{
	uint32_t v;

	asm volatile (
		"inl %1, %0"
		:"=a" (v)
		:"dN"(port)
		);
	return v;
}

void pio_write32(uint32_t v, uint16_t port)
{
	asm volatile (
		"outl %0, %1"
		:
		:"a" (v), "dN"(port)
		);
}

