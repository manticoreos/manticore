#include <arch/setup.h>

#include <kernel/page-alloc.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <kernel/mmu.h>
#include <kernel/pci.h>

#include <arch/exceptions.h>
#include <arch/syscall.h>
#include <arch/segment.h>
#include <arch/i8259.h>
#include <arch/task.h>
#include <arch/cpu.h>
#include <arch/gdt.h>
#include <arch/msr.h>

#include <string.h>
#include <stdint.h>

#define GDT_ENTRY(value) (((uint64_t) value) << 32)

uint64_t gdt[] __attribute__ ((aligned (8))) = {
	0,
	/* Kernel code segment: */
	GDT_ENTRY(X86_GDT_TYPE_CODE | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_L),
	/* Kernel data segment: */
	GDT_ENTRY(X86_GDT_TYPE_DATA | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_DB),

	/* Userspace data segment: */
	GDT_ENTRY(X86_GDT_TYPE_DATA | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(3) | X86_GDT_DB),
	/* Userspace code segment: */
	GDT_ENTRY(X86_GDT_TYPE_CODE | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(3) | X86_GDT_L),

	/* TSS: */
	0,
	0,
};

struct gdt_desc64 gdt_desc = {
	.limit	= ARRAY_SIZE(gdt) * sizeof(uint64_t) - 1,
	.base	= (uint64_t) gdt,
};

static void init_gdt(void)
{
	asm volatile(
		"lgdt %0\n"
		:
		: "m"(gdt_desc)
		: "memory");
}

void init_mmu_map(void)
{
	void *page = page_alloc_small();
	if (!page) {
		panic("Unable to allocate kernel MMU map");
	}
	memset(page, 0, PAGE_SIZE_SMALL);
	mmu_map_t mmu_map = ptr_to_paddr(page);
	for (unsigned i = 0; i < nr_mem_regions; i++) {
		struct memory_region *mem_region = &mem_regions[i];
		int err = mmu_map_range(mmu_map, mem_region->base + KERNEL_VMA, mem_region->base, mem_region->len, 0);
		if (err) {
			panic("Unable to setup kernel MMU map");
		}
	}
	mmu_load_map(mmu_map);
	mmu_invalidate_tlb();
}

static void setup_nxe(void)
{
	wrmsr(X86_IA32_EFER, rdmsr(X86_IA32_EFER) | X86_IA32_EFER_NXE);
}

void arch_early_setup(void)
{
	i8259_remap();
	init_gdt();
	init_idt();
	init_task();
	init_syscall();
	init_memory_map();
	init_mmu_map();
	setup_nxe();
}

void arch_late_setup(void)
{
	pci_probe();
}
