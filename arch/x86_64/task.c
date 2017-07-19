#include <arch/segment.h>
#include <arch/gdt.h>

#include <stdint.h>

struct tss {
	uint32_t	reserved_0;
	uint64_t	rsp[3];
	uint64_t	reserved_1;
	uint64_t	ist[7];
	uint64_t	reserved_2;
	uint16_t	reserved_3;
	uint16_t	io_map_base;
} __attribute__ ((packed));

struct tss_desc {
	uint16_t	segment_limit_1;
	uint16_t	base_addr_1;
	uint8_t		base_addr_2;
	unsigned	type : 4;
	unsigned	system : 1;
	unsigned	dpl : 2;
	unsigned	present : 1;
	unsigned	segment_limit_2 : 4;
	unsigned	avl : 1;
	unsigned	reserved_0 : 2;
	unsigned 	gran : 1;
	uint8_t		base_addr_3;
	uint32_t	base_addr_4;
	uint32_t	reserved_1;
} __attribute__ ((packed));

static void init_tss_desc(struct tss_desc *desc, uint64_t base, uint32_t limit)
{
	desc->segment_limit_1 = (limit & 0x0ffff);
	desc->segment_limit_2 = (limit & 0xf00000) >> 16;
	desc->gran = 0;
	desc->base_addr_1 = (base & 0x000000000000ffff);
	desc->base_addr_2 = (base & 0x0000000000ff0000) >> 16;
	desc->base_addr_3 = (base & 0x00000000ff000000) >> 24;
	desc->base_addr_4 = (base & 0xffffffff00000000) >> 32;
	desc->type = 9;
	desc->system = 0;
	desc->dpl = 0;
	desc->present = 1;
	desc->avl = 0;
	desc->reserved_0 = 0;
	desc->reserved_1 = 0;
}

static void init_tss(struct tss *tss, void *rsp, void *ist)
{
	tss->reserved_0 = 0;
	tss->rsp[0] = (uint64_t) rsp;
	tss->rsp[1] = 0;
	tss->rsp[2] = 0;
	tss->reserved_1 = 0;
	tss->ist[0] = (uint64_t) ist;
	tss->ist[1] = 0;
	tss->ist[2] = 0;
	tss->ist[3] = 0;
	tss->ist[4] = 0;
	tss->ist[5] = 0;
	tss->ist[6] = 0;
	tss->reserved_2 = 0;
	tss->reserved_3 = 0;
	tss->io_map_base = 0;
}

static struct tss tss;
static char kernel_stack[4096] __attribute__ ((aligned (16)));
static char exception_stack[4096] __attribute__ ((aligned (16)));

static void set_cs(uint64_t cs)
{
	asm volatile(
		"pushq %0\n"
		"leaq  1f(%%rip), %%rax\n"
		"pushq %%rax\n"
		"lretq\n"
		"1:\n"
		:
		: "r"(cs)
		: "memory", "rax"
		);
}

static void load_task_reg(struct tss *tss)
{
	struct tss_desc *tss_desc = (struct tss_desc *) &gdt[X86_GDT_TSS_IDX];
	init_tss_desc(tss_desc, (uint64_t) tss, sizeof(*tss));
	asm volatile("lgdt %0" : : "m" (gdt_desc));
	set_cs(X86_KERNEL_CS);
	asm volatile("ltr %w0" : : "r" (X86_GDT_TSS_IDX * 8));
}

void init_task(void)
{
	init_tss(&tss, kernel_stack + 4096, exception_stack + 4096);

	load_task_reg(&tss);
}
