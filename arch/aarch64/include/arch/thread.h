#ifndef ARM64_THREAD_H
#define ARM64_THREAD_H

#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////
// \breif the file descives the task_state structre and functions for initalising
//        task_state_init and task changes state
///////////////////////////////////////////////////////////////////////////////

struct task_state {
	void *sp;
	void *pc;
};

void task_state_init(struct task_state *task_state, void *pc, void *sp);
void switch_to(struct task_state *old, struct task_state *new);

#endif
