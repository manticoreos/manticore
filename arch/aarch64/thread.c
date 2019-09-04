#include <arch/thread.h>


/// The task_state_init function enables task switching that is
/// converts task state in a process control block and updates
/// the program counter and defines sp
///
/// \param task_state A variable of type task_state struct object
/// \param pc         Program counter
/// \param sp         Stack pointer
/// \returns          returns the task state
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
