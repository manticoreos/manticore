#ifndef ARCH_IOPORT_H
#define ARCH_IOPORT_H

#include <stdint.h>

uint8_t inb(uint16_t port);

void outb(uint8_t v, uint16_t port);

uint16_t inw(uint16_t port);

void outw(uint16_t v, uint16_t port);

uint32_t inl(uint16_t port);

void outl(uint32_t v, uint16_t port);

#endif
