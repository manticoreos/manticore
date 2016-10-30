#ifndef X86_THREAD_H
#define X86_THREAD_H

#include <stddef.h>

struct task_state {
	void *rsp;
	void *rip;
};

void task_state_init(struct task_state *task_state, void *rip, void *rsp);
void switch_to(struct task_state *old, struct task_state *new);

#endif
