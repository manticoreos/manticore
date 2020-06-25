#include <arch/thread.h>

#include <kernel/kmem.h>

void task_state_init(struct task_state *task_state, void *pc, void *sp)
{
	task_state->sp = sp;
	task_state->pc = pc;
}

struct task_state *task_state_new(void *pc, void *sp)
{
	struct task_state *ret = kmem_alloc(sizeof(struct task_state));
	if (ret) {
		ret->flags = TIF_NEW;
		ret->pc = pc;
		ret->sp = sp;
	}
	return ret;
}

void task_state_delete(struct task_state *task_state)
{
	kmem_free(task_state, sizeof(struct task_state));
}

void *task_state_stack_top(struct task_state *task_state)
{
	return task_state->sp;
}

void *task_state_entry_point(struct task_state *task_state)
{
	return task_state->pc;
}
void switch_to(struct task_state *old, struct task_state *new)
{
	asm volatile(
		"mov	x9, sp\n"
		"adr	x10, 0f\n"
		"stp	x9, x10, %0\n"

		"ldp	x9, x10, %1\n"
		"mov	sp, x9\n"
		"br	x10\n"
		"0:\n"
		:
		: "m"(old->sp), "m"(new->sp)
		: "x9", "x10", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x30",
		  "memory");
}
