#ifndef KERNEL_ALIGN_H
#define KERNEL_ALIGN_H

#include <stdbool.h>
#include <stdint.h>


/// is_aligned checks for alignment
/// \param size  integer of 64 bit size
/// \param align unsigned integer having the alignment
/// \return      if size is aligned that is 0 if remainder is zero.

static inline bool is_aligned(uint64_t size, uint64_t align)
{
	return (size % align) == 0;
}

static inline uint64_t align_down(uint64_t size, uint64_t align)
{
       return size & ~(align - 1);
}

static inline uint64_t align_up(uint64_t size, uint64_t align)
{
       return align_down(size + align - 1, align);
}

#endif
