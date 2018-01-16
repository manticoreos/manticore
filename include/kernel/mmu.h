#ifndef KERNEL_MMU_H
#define KERNEL_MMU_H 1

#include <arch/vmem.h>

#include <stddef.h>
#include <stdint.h>

typedef enum {
	MMU_USER_PAGE = 1UL << 0,
} mmu_flags_t;

/// MMU translation map
typedef uintptr_t mmu_map_t;

void mmu_invalidate_tlb(void);
mmu_map_t mmu_current_map(void);
void mmu_load_map(mmu_map_t map);
int mmu_map_range(mmu_map_t map, virt_t vaddr, phys_t paddr, size_t size, mmu_flags_t flags);
int mmu_map_small_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_flags_t flags);
int mmu_map_large_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_flags_t flags);
void mmu_map_dump(mmu_map_t map);

#endif
