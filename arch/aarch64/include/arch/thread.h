#ifndef ARM64_THREAD_H
#define ARM64_THREAD_H

#include <stddef.h>

struct task_state {
	void *sp;
	void *pc;
};

void task_state_init(struct task_state *task_state, void *pc, void *sp);
void switch_to(struct task_state *old, struct task_state *new);

#endif
