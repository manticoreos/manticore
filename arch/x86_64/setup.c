#include <arch/setup.h>

#include <arch/exceptions.h>
#include <arch/segment.h>

#include <stdint.h>

struct gdt_desc {
	uint16_t	limit;
	void		*base;
} __attribute__ ((packed));

#define GDT_ENTRY(value) (((uint64_t) value) << 32)

static uint64_t gdt[] __attribute__ ((aligned (8))) = {
	0,
	/* Kernel code segment: */
	GDT_ENTRY(X86_GDT_TYPE_CODE | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_L),
	/* Kernel data segment: */
	GDT_ENTRY(X86_GDT_TYPE_DATA | X86_GDT_P | X86_GDT_S | X86_GDT_DPL(0) | X86_GDT_DB),
};

static struct gdt_desc gdt_desc = {
	.limit	= ARRAY_SIZE(gdt) * sizeof(uint64_t) - 1,
	.base	= gdt,
};

static void init_gdt(void)
{
	asm volatile(
		"lgdt %0\n"
		:
		: "m"(gdt_desc)
		: "memory");
}

void arch_setup(void)
{
	init_gdt();
	init_idt();
}
