#include <stdint.h>

uint8_t mmio_read8(uint64_t addr)
{
	return *((volatile uint8_t *)addr);
}

void mmio_write8(uint8_t value, uint64_t addr)
{
	*((volatile uint8_t *)addr) = value;
}

uint16_t mmio_read16(uint64_t addr)
{
	return *((volatile uint16_t *)addr);
}

void mmio_write16(uint16_t value, uint64_t addr)
{
	*((volatile uint16_t *)addr) = value;
}

uint32_t mmio_read32(uint64_t addr)
{
	return *((volatile uint32_t *)addr);
}

void mmio_write32(uint32_t value, uint64_t addr)
{
	*((volatile uint32_t *)addr) = value;
}

uint64_t mmio_read64(uint64_t addr)
{
	return *((volatile uint64_t *)addr);
}

void mmio_write64(uint64_t value, uint64_t addr)
{
	*((volatile uint64_t *)addr) = value;
}
