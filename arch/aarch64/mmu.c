#include <kernel/mmu.h>

#include <kernel/errno.h>

phys_t virt_to_phys(virt_t addr)
{
	return addr;
}

mmu_map_t mmu_current_map(void)
{
	/* FIXME: not implemented.  */
	return 0;
}

void mmu_load_map(mmu_map_t map)
{
	/* FIXME: not implemented.  */
}

int mmu_map_small_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_prot_t prot, mmu_flags_t flags)
{
	/* FIXME: not implemented.  */
	return -EINVAL;
}

int mmu_map_large_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_prot_t prot, mmu_flags_t flags)
{
	/* FIXME: not implemented.  */
	return -EINVAL;
}
