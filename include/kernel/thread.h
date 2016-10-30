#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include <arch/thread.h>

struct thread {
	struct task_state task_state;
};

typedef void (*thread_start_t)(void);

int thread_create(struct thread *thread, thread_start_t, void *stack_top);

#endif
