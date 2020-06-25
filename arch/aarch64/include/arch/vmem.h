#ifndef AARCH64_VMEM_H
#define AARCH64_VMEM_H

#include <arch/vmem-defs.h>

#include <stdint.h>

/// Physical address.
typedef uint64_t phys_t;

/// Virtual address.
typedef uint64_t virt_t;

/// Converts a virtual address to a physical address.
phys_t virt_to_phys(virt_t addr);

#endif
