#include <arch/thread.h>

void task_state_init(struct task_state *task_state, void *rip, void *rsp)
{
	task_state->rsp = rsp;
	task_state->rip = rip;
}

void switch_to(struct task_state *old, struct task_state *new)
{
	asm volatile(
		"push	%%rbp\n"
		"movq	$0f, %c[rip](%0)\n"
		"movq	%%rsp, %c[rsp](%0)\n"
		"movq	%c[rsp](%1), %%rsp\n"
		"jmpq	*%c[rip](%1)\n"
		"0:\n"
		"pop	%%rbp\n"
		:
		: "D"(old), "S"(new),
		  [rsp]"i"(offsetof(struct task_state, rsp)),
		  [rip]"i"(offsetof(struct task_state, rip))
		: "rax", "rbx", "rcx", "rdx", "rbp", "rsp",
		  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
		  "memory", "flags", "cc");
}
