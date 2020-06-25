#include <kernel/mmu.h>

#include <kernel/align.h>
#include <kernel/errno.h>
#include <kernel/page-alloc.h>

/// Maps virtual address range to a physical address range.
///
/// This function does not invalidate TLB. Callers are expected to do that by calling
/// mmu_invalidate_tlb().
//
/// \param map MMU translation map.
/// \param vaddr Start of virtual address range to map from.
/// \param paddr Start of physical address range to map to.
/// \param size Size of the address range to map.
/// \param flags MMU flags for the mapped address range.
/// \param prot MMU protection for the mapped address range.
///
/// \return 0 if successful
///         -EINVAL if passed parameters are invalid
///         -ENOMEM if out of memory
int mmu_map_range(mmu_map_t map, virt_t vaddr, phys_t paddr, size_t size, mmu_prot_t prot, mmu_flags_t flags)
{
	if (!is_aligned(vaddr, PAGE_SIZE_SMALL)) {
		return -EINVAL;
	}
	virt_t start = vaddr;
	virt_t end = vaddr + size;
	virt_t large_start = align_up(vaddr, PAGE_SIZE_LARGE);
	virt_t large_end = align_down(vaddr + size, PAGE_SIZE_LARGE);
	if (large_end < large_start) {
		/* No large pages: */
		for (virt_t offset = start; offset < end; offset += PAGE_SIZE_SMALL) {
			int err = mmu_map_small_page(map, offset, paddr, prot, flags);
			if (err) {
				return err;
			}
			paddr += PAGE_SIZE_SMALL;
		}
	} else {
		/* First map any small pages before the large pages: */
		if (start != large_start) {
			for (virt_t offset = start; offset < large_start; offset += PAGE_SIZE_SMALL) {
				int err = mmu_map_small_page(map, offset, paddr, prot, flags);
				if (err) {
					return err;
				}
				paddr += PAGE_SIZE_SMALL;
			}
		}
		/* Then map the large pages: */
		if (large_start != large_end) {
			for (virt_t offset = large_start; offset < large_end; offset += PAGE_SIZE_LARGE) {
				int err = mmu_map_large_page(map, offset, paddr, prot, flags);
				if (err) {
					return err;
				}
				paddr += PAGE_SIZE_LARGE;
			}
		}
		/* Finally, map left-over small pages: */
		if (large_end != end) {
			for (virt_t offset = large_end; offset < end; offset += PAGE_SIZE_SMALL) {
				int err = mmu_map_small_page(map, offset, paddr, prot, flags);
				if (err) {
					return err;
				}
				paddr += PAGE_SIZE_SMALL;
			}
		}
	}
	return 0;
}
