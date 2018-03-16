#include <kernel/page-alloc.h>
#include <kernel/console.h>
#include <kernel/initrd.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <kernel/panic.h>
#include <kernel/kmem.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>
#include <arch/setup.h>

#include <stddef.h>

void start_kernel(void)
{
	int err;
	console_init();
	printf("Booting kernel ...\n");
	page_alloc_init();
	arch_early_setup();
	err = kmem_init();
	if (err) {
		panic("kmem_init failed");
	}
	arch_late_setup();
	arch_local_interrupt_enable();
	initrd_load();
#ifdef HAVE_TEST
	test_kmem();
	test_page_alloc();
#endif
	printf("Halted.\n");
	arch_halt_cpu();
}
