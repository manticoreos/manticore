#ifndef KERNEL_ALIGN_H
#define KERNEL_ALIGN_H

#include <stdint.h>

static inline uint64_t align_down(uint64_t size, uint64_t align)
{
       return size & ~(align - 1);
}

static inline uint64_t align_up(uint64_t size, uint64_t align)
{
       return align_down(size + align - 1, align);
}

#endif
