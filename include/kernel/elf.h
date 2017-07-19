#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

#include <stdint.h>

void *parse_elf(uint64_t start, uint64_t end);

#endif
