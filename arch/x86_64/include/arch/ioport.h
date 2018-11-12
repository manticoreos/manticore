#ifndef ARCH_IOPORT_H
#define ARCH_IOPORT_H

#include <stdint.h>

uint8_t pio_read8(uint16_t port);

void pio_write8(uint8_t v, uint16_t port);

uint16_t pio_read16(uint16_t port);

void pio_write16(uint16_t v, uint16_t port);

uint32_t pio_read32(uint16_t port);

void pio_write32(uint32_t v, uint16_t port);

#endif
