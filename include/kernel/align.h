#ifndef KERNEL_ALIGN_H
#define KERNEL_ALIGN_H

#include <stdbool.h>
#include <stdint.h>

/// Check if \size is aligned to \align.
///
/// \param size  The size to check for alignment.
/// \param align The alignment to check for.
/// \returns true if \size is aligned to \align; otherwise returns false.
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
