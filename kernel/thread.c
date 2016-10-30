#include <kernel/thread.h>

int thread_create(struct thread *thread, thread_start_t start_fn, void *stack_top)
{
	task_state_init(&thread->task_state, (void *) start_fn, stack_top);

	return 0;
}
