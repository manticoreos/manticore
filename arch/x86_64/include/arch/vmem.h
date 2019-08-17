#ifndef ARCH_VMEM_H
#define ARCH_VMEM_H

#include <arch/vmem-defs.h>

#include <stdint.h>

/// Physical address.
typedef uint64_t phys_t;

/// Virtual address.
typedef uint64_t virt_t;

/// Converts a virtual address to a physical address.
phys_t virt_to_phys(virt_t addr);

/// Converts a physical address to a virtual address.
virt_t phys_to_virt(phys_t addr);

/// Converts a virtual address pointer to physical address.
static inline phys_t ptr_to_paddr(void *page)
{
	return virt_to_phys((virt_t)page);
}

/// Converts a physical address to a virtual address pointer.
static inline void *paddr_to_ptr(phys_t paddr)
{
	return (void *)(paddr + KERNEL_VMA);
}

#endif
