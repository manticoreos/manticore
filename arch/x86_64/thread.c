#include <arch/thread.h>

#include <kernel/kmem.h>

void task_state_init(struct task_state *task_state, void *rip, void *rsp)
{
	task_state->rsp = rsp;
	task_state->rip = rip;
}

struct task_state *task_state_new(void *rip, void *rsp)
{
	struct task_state *ret = kmem_alloc(sizeof(struct task_state));
	if (ret) {
		ret->flags = TIF_NEW;
		ret->rip = rip;
		ret->rsp = rsp;
	}
	return ret;
}

void task_state_delete(struct task_state *task_state)
{
	kmem_free(task_state, sizeof(struct task_state));
}

void *task_state_stack_top(struct task_state *task_state)
{
	return task_state->rsp;
}

void *task_state_entry_point(struct task_state *task_state)
{
	return task_state->rip;
}

void switch_to_first(struct task_state *new)
{
	asm volatile(
		"push	%%rbp\n"
		"btrl	%[tif_new], %c[flags](%0)\n"
		"movq	%c[rip](%0), %%rdi\n"
		"movq	%c[rsp](%0), %%rsi\n"
		"jc	ret_to_userspace\n"
		"movq	%%rsi, %%rsp\n"
		"jmpq	*%%rdi\n"
		"0:\n"
		"pop	%%rbp\n"
		:
		: "S"(new),
		  [rsp]"i"(offsetof(struct task_state, rsp)),
		  [rip]"i"(offsetof(struct task_state, rip)),
		  [tif_new]"i"(__TIF_NEW),
		  [flags]"i"(offsetof(struct task_state, flags))
		: "rax", "rbx", "rcx", "rdx", "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
		  "memory", "flags", "cc");
}

void switch_to(struct task_state *old, struct task_state *new)
{
	asm volatile(
		"push	%%rbp\n"
		"movq	$0f, %c[rip](%0)\n"
		"movq	%%rsp, %c[rsp](%0)\n"
		"btrl	%[tif_new], %c[flags](%1)\n"
		"movq	%c[rip](%1), %%rdi\n"
		"movq	%c[rsp](%1), %%rsi\n"
		"jc	ret_to_userspace\n"
		"movq	%%rsi, %%rsp\n"
		"jmpq	*%%rdi\n"
		"0:\n"
		"pop	%%rbp\n"
		:
		: "D"(old), "S"(new),
		  [rsp]"i"(offsetof(struct task_state, rsp)),
		  [rip]"i"(offsetof(struct task_state, rip)),
		  [tif_new]"i"(__TIF_NEW),
		  [flags]"i"(offsetof(struct task_state, flags))
		: "rax", "rbx", "rcx", "rdx", "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
		  "memory", "flags", "cc");
}
