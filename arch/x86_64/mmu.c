#include <kernel/mmu.h>

#include <kernel/align.h>
#include <kernel/errno.h>
#include <kernel/page-alloc.h>
#include <kernel/printf.h>

#include <arch/vmem.h>
#include <arch/mmu.h>

#include <string.h>

phys_t virt_to_phys(virt_t addr)
{
	return addr - KERNEL_VMA;
}

// Entries in paging structure:
#define NR_PG_ENTRIES 512
// Bits used for translation per level:
#define PG_TABLE_BITS 9
// Mask for translation per level:
#define PG_INDEX_MASK ((1ULL << PG_TABLE_BITS) - 1)

// Page Map Level 4 (PML4)
#define PML4_INDEX_SHIFT 39
#define PML4_INDEX_MASK PG_INDEX_MASK
// Page Directory Pointer Table (PDPT)
#define PDPT_INDEX_SHIFT 30
#define PDPT_INDEX_MASK PG_INDEX_MASK
// Page Directory (PD)
#define PD_INDEX_SHIFT 21
#define PD_INDEX_MASK PG_INDEX_MASK
// Page Table (PT)
#define PT_INDEX_SHIFT 12
#define PT_INDEX_MASK PG_INDEX_MASK

// Page Map Level 4 Entry (PML4E)
#define PML4E_FLAGS_MASK 0x80000000000007ffULL
#define PML4E_PADDR_MASK (~PML4E_FLAGS_MASK)
// Page Directory Pointer Table Entry (PDPTE)
#define PDPTE_FLAGS_MASK 0x80000000000007ffULL
#define PDPTE_PADDR_MASK (~PDPTE_FLAGS_MASK)
// Page Directory Entry (PDE)
#define PDE_FLAGS_MASK 0x80000000000007ffULL
#define PDE_PADDR_MASK (~PDE_FLAGS_MASK)
// Page Table Entry (PTE)
#define PTE_FLAGS_MASK 0x80000000000007ffULL
#define PTE_PADDR_MASK (~PTE_FLAGS_MASK)

typedef struct {
	uint64_t pml4e;
} pml4e_t;

typedef struct {
	uint64_t pdpte;
} pdpte_t;

typedef struct {
	uint64_t pde;
} pde_t;

typedef struct {
	uint64_t pte;
} pte_t;

static pml4e_t make_pml4e(phys_t paddr, uint64_t flags)
{
	return (pml4e_t){pml4e: paddr | flags};
}

static pdpte_t make_pdpte(phys_t paddr, uint64_t flags)
{
	return (pdpte_t){pdpte: paddr | flags};
}

static pde_t make_pde(phys_t paddr, uint64_t flags)
{
	return (pde_t){pde: paddr | flags};
}

static pte_t make_pte(phys_t paddr, uint64_t flags)
{
	return (pte_t){pte: paddr | flags};
}

static bool pml4e_is_none(pml4e_t pml4e)
{
	return !pml4e.pml4e;
}

static bool pdpte_is_none(pdpte_t pdpte)
{
	return !pdpte.pdpte;
}

static bool pde_is_none(pde_t pde)
{
	return !pde.pde;
}

static bool pte_is_none(pte_t pte)
{
	return !pte.pte;
}

static bool pde_is_large(pde_t pde)
{
	return pde.pde & X86_PE_PS;
}

static uint64_t pml4e_paddr(pml4e_t pml4e)
{
	return pml4e.pml4e & PML4E_PADDR_MASK;
}

static uint64_t pdpte_paddr(pdpte_t pdpte)
{
	return pdpte.pdpte & PDPTE_PADDR_MASK;
}

static uint64_t pde_paddr(pde_t pde)
{
	return pde.pde & PDE_PADDR_MASK;
}

static uint64_t pte_paddr(pte_t pte)
{
	return pte.pte & PTE_PADDR_MASK;
}

static uint64_t pml4e_flags(pml4e_t pml4e)
{
	return pml4e.pml4e & PML4E_FLAGS_MASK;
}

static uint64_t pdpte_flags(pdpte_t pdpte)
{
	return pdpte.pdpte & PDPTE_FLAGS_MASK;
}

static uint64_t pde_flags(pde_t pde)
{
	return pde.pde & PDE_FLAGS_MASK;
}

static uint64_t pte_flags(pte_t pte)
{
	return pte.pte & PTE_FLAGS_MASK;
}

static uint64_t x86_read_cr3(void)
{
	uint64_t ret;
	asm volatile(
		"mov %%cr3, %0"
		: "=r"(ret));
	return ret;
}

static void x86_write_cr3(uint64_t value)
{
	asm volatile(
		"mov %0, %%cr3"
		:
		: "r"(value));
}

void mmu_invalidate_tlb(void)
{
	x86_write_cr3(x86_read_cr3());
}

mmu_map_t mmu_current_map(void)
{
	return x86_read_cr3();
}

void mmu_load_map(mmu_map_t map)
{
	x86_write_cr3(map);
}

/// Converts paging structure indices to a virtual address.
static virt_t pg_index_to_vaddr(uint64_t pml4_idx, uint64_t pdpt_idx, uint64_t pd_idx,
				uint64_t pt_idx)
{
	return (pml4_idx << PML4_INDEX_SHIFT) | (pdpt_idx << PDPT_INDEX_SHIFT) |
	       (pd_idx << PD_INDEX_SHIFT) | (pt_idx << PT_INDEX_SHIFT);
}

static uint64_t mmu_prot_to_hw(mmu_prot_t prot)
{
	uint64_t hw_flags = X86_PE_P;
	if (prot & MMU_PROT_WRITE) {
		hw_flags |= X86_PE_RW;
	}
	if (!(prot & MMU_PROT_EXEC)) {
		hw_flags |= X86_PE_XD;
	}
	return hw_flags;
}

static uint64_t mmu_flags_to_hw(mmu_flags_t flags)
{
	uint64_t hw_flags = 0;
	if (flags & MMU_USER_PAGE) {
		hw_flags |= X86_PE_US;
	}
	return hw_flags;
}

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
	if (start != large_start) {
		for (virt_t offset = start; offset < large_start; offset += PAGE_SIZE_SMALL) {
			int err = mmu_map_small_page(map, offset, paddr, prot, flags);
			if (err) {
				return err;
			}
			paddr += PAGE_SIZE_SMALL;
		}
	}
	if (large_start != large_end) {
		for (virt_t offset = large_start; offset < large_end; offset += PAGE_SIZE_LARGE) {
			int err = mmu_map_large_page(map, offset, paddr, prot, flags);
			if (err) {
				return err;
			}
			paddr += PAGE_SIZE_LARGE;
		}
	}
	if (large_end != end) {
		for (virt_t offset = large_end; offset < end; offset += PAGE_SIZE_SMALL) {
			int err = mmu_map_small_page(map, offset, paddr, prot, flags);
			if (err) {
				return err;
			}
			paddr += PAGE_SIZE_SMALL;
		}
	}
	return 0;
}

int mmu_map_small_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_prot_t prot, mmu_flags_t flags)
{
	uint64_t hw_flags = mmu_flags_to_hw(flags);
	pml4e_t *pml4_table = paddr_to_ptr(map);
	uint64_t pml4_idx = (vaddr >> PML4_INDEX_SHIFT) & PML4_INDEX_MASK;
	pml4e_t pml4e = pml4_table[pml4_idx];
	if (pml4e_is_none(pml4e)) {
		void *pdp_page = page_alloc_small();
		if (!pdp_page) {
			return -ENOMEM;
		}
		memset(pdp_page, 0, PAGE_SIZE_SMALL);
		pml4e = make_pml4e(ptr_to_paddr(pdp_page), X86_PE_P | X86_PE_RW | hw_flags);
		pml4_table[pml4_idx] = pml4e;
	}
	pdpte_t *pdp_table = paddr_to_ptr(pml4e_paddr(pml4e));
	uint64_t pdpt_idx = (vaddr >> PDPT_INDEX_SHIFT) & PDPT_INDEX_MASK;
	pdpte_t pdpte = pdp_table[pdpt_idx];
	if (pdpte_is_none(pdpte)) {
		void *pd_page = page_alloc_small();
		if (!pd_page) {
			return -ENOMEM;
		}
		memset(pd_page, 0, PAGE_SIZE_SMALL);
		pdpte = make_pdpte(ptr_to_paddr(pd_page), X86_PE_P | X86_PE_RW | hw_flags);
		pdp_table[pdpt_idx] = pdpte;
	}
	pde_t *pd = paddr_to_ptr(pdpte_paddr(pdpte));
	uint64_t pd_idx = (vaddr >> PD_INDEX_SHIFT) & PD_INDEX_MASK;
	pde_t pde = pd[pd_idx];
	if (pde_is_none(pde)) {
		void *pt_page = page_alloc_small();
		if (!pt_page) {
			return -ENOMEM;
		}
		memset(pt_page, 0, PAGE_SIZE_SMALL);
		pde = make_pde(ptr_to_paddr(pt_page), X86_PE_P | X86_PE_RW | hw_flags);
		pd[pd_idx] = pde;
	}
	if (pde_is_large(pde)) {
		/* PDE is already mapped as a large page. */
		return -EINVAL;
	}
	uint64_t hw_prot = mmu_prot_to_hw(prot);
	pte_t *pt = paddr_to_ptr(pde_paddr(pde));
	uint64_t pt_idx = (vaddr >> PT_INDEX_SHIFT) & PT_INDEX_MASK;
	pt[pt_idx] = make_pte(paddr, hw_prot | hw_flags);
	return 0;
}

int mmu_map_large_page(mmu_map_t map, virt_t vaddr, phys_t paddr, mmu_prot_t prot, mmu_flags_t flags)
{
	uint64_t hw_flags = mmu_flags_to_hw(flags);
	pml4e_t *pml4_table = paddr_to_ptr(map);
	uint64_t pml4_idx = (vaddr >> PML4_INDEX_SHIFT) & PML4_INDEX_MASK;
	pml4e_t pml4e = pml4_table[pml4_idx];
	if (pml4e_is_none(pml4e)) {
		void *pdp_page = page_alloc_small();
		if (!pdp_page) {
			return -ENOMEM;
		}
		memset(pdp_page, 0, PAGE_SIZE_SMALL);
		pml4e = make_pml4e(ptr_to_paddr(pdp_page), X86_PE_P | X86_PE_RW | hw_flags);
		pml4_table[pml4_idx] = pml4e;
	}
	pdpte_t *pdp_table = paddr_to_ptr(pml4e_paddr(pml4e));
	uint64_t pdpt_idx = (vaddr >> PDPT_INDEX_SHIFT) & PDPT_INDEX_MASK;
	pdpte_t pdpte = pdp_table[pdpt_idx];
	if (pdpte_is_none(pdpte)) {
		void *pd_page = page_alloc_small();
		if (!pd_page) {
			return -ENOMEM;
		}
		memset(pd_page, 0, PAGE_SIZE_SMALL);
		pdpte = make_pdpte(ptr_to_paddr(pd_page), X86_PE_P | X86_PE_RW | hw_flags);
		pdp_table[pdpt_idx] = pdpte;
	}
	pde_t *pd = paddr_to_ptr(pdpte_paddr(pdpte));
	uint64_t pd_idx = (vaddr >> PD_INDEX_SHIFT) & PD_INDEX_MASK;
	pde_t pde = pd[pd_idx];
	if (!pde_is_none(pde) && !pde_is_large(pde)) {
		/* PDE is already mapped as a page table. */
		return -EINVAL;
	}
	uint64_t hw_prot = mmu_prot_to_hw(prot);
	pd[pd_idx] = make_pde(paddr, hw_prot | hw_flags | X86_PE_PS);
	return 0;
}

static void mmu_dump_pde(unsigned pml4_idx, unsigned pdpt_idx, unsigned pd_idx, pde_t pde)
{
	if (pde_is_large(pde)) {
		printf("      PDE: %4d %016lx -> %016lx [%lx]\n", pd_idx,
		       pg_index_to_vaddr(pml4_idx, pdpt_idx, pd_idx, 0), pde_paddr(pde),
		       pde_flags(pde));
	} else {
		printf("      PDE: %d @ %016lx [%lx]\n", pd_idx, pde_paddr(pde), pde_flags(pde));

		pte_t *pt = paddr_to_ptr(pde_paddr(pde));
		for (unsigned pt_idx = 0; pt_idx < NR_PG_ENTRIES; pt_idx++) {
			pte_t pte = pt[pt_idx];
			if (pte_is_none(pte)) {
				continue;
			}
			printf("      PTE: %4d %016lx -> %016lx [%lx]\n", pd_idx,
			       pg_index_to_vaddr(pml4_idx, pdpt_idx, pd_idx, pt_idx),
			       pte_paddr(pte), pte_flags(pte));
		}
	}
}

static void mmu_dump_pdpte(unsigned pml4_idx, unsigned pdpt_idx, pdpte_t pdpte)
{
	printf("    PDPTE: %d @ %016lx [%lx]\n", pdpt_idx, pdpte_paddr(pdpte), pdpte_flags(pdpte));

	pde_t *pd = paddr_to_ptr(pdpte_paddr(pdpte));
	for (unsigned pd_idx = 0; pd_idx < NR_PG_ENTRIES; pd_idx++) {
		pde_t pde = pd[pd_idx];
		if (pde_is_none(pde)) {
			continue;
		}
		mmu_dump_pde(pml4_idx, pdpt_idx, pd_idx, pde);
	}
}

static void mmu_dump_pml4e(unsigned pml4_idx, pml4e_t pml4e)
{
	printf("  PML4E: %d @ %016lx [%lx]\n", pml4_idx, pml4e_paddr(pml4e), pml4e_flags(pml4e));

	pdpte_t *pdp_table = paddr_to_ptr(pml4e_paddr(pml4e));
	for (unsigned pdpt_idx = 0; pdpt_idx < NR_PG_ENTRIES; pdpt_idx++) {
		pdpte_t pdpte = pdp_table[pdpt_idx];
		if (pdpte_is_none(pdpte)) {
			continue;
		}
		mmu_dump_pdpte(pml4_idx, pdpt_idx, pdpte);
	}
}

void mmu_map_dump(mmu_map_t map)
{
	pml4e_t *pml4_table = paddr_to_ptr(map);
	printf("PML4 table: %p\n", pml4_table);

	for (unsigned pml4_idx = 0; pml4_idx < NR_PG_ENTRIES; pml4_idx++) {
		pml4e_t pml4e = pml4_table[pml4_idx];
		if (pml4e_is_none(pml4e)) {
			continue;
		}
		mmu_dump_pml4e(pml4_idx, pml4e);
	}
}
