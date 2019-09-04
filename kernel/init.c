#include <kernel/page-alloc.h>
#include <kernel/console.h>
#include <kernel/initrd.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/kmem.h>
#include <kernel/cpu.h>

#include <arch/interrupts.h>
#include <arch/thread.h>
#include <arch/setup.h>

#include <stddef.h>

#define IDLE_STACK_SIZE PAGE_SIZE_SMALL

static char idle_stack[IDLE_STACK_SIZE] __attribute__((aligned(16)));
struct task_state *idle_task;

static void idle(void)
{
	for (;;) {
		arch_halt_cpu();
		wake_up_processes();
		schedule();
	}
}


/// The function start_kernel initializes and starts the kernel for boot.

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
	test_printf();
#endif
	idle_task = task_state_new(idle, idle_stack + IDLE_STACK_SIZE);
	if (!idle_task) {
		panic("unable to allocate idle task");
	}
	idle_task->flags = 0;
	schedule();
	printf("Halted.\n");
	arch_halt_cpu();
}
