#include <arch/thread.h>

/////////////////////////////////////////////////////////////////////////////
// \breif the function convers task state in a process control block, changes
//        the program counter and defines sp
//
// \param takes struct task_state and information that should be updated
// \returns function accepts a structure pointers
void task_state_init(struct task_state *task_state, void *pc, void *sp)
{
	task_state->sp = sp;
	task_state->pc = pc;
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
