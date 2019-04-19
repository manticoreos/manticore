#ifndef X86_THREAD_H
#define X86_THREAD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//! The TIF_NEW flag represents a task that has never been scheduled.
//! The switch_to() function takes this into account and switches the task
//! to userspace context.
#define __TIF_NEW	0
#define TIF_NEW		(1ULL << __TIF_NEW)

struct task_state {
	void *rsp;
	void *rip;
	uint32_t flags;
};

struct task_state *task_state_new(void *rip, void *rsp);
void task_state_delete(struct task_state *task_state);
void task_state_init(struct task_state *task_state, void *rip, void *rsp);
void *task_state_stack_top(struct task_state *task_state);
void *task_state_entry_point(struct task_state *task_state);

void switch_to(struct task_state *old, struct task_state *new);
void switch_to_first(struct task_state *task_state);
void ret_to_userspace(void *rip, void *rsp);

#endif
