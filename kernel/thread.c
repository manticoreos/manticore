#include <kernel/thread.h>

/// thread_create creates the thread
/// \param thread         thread is of struct thread type storing thread information
/// \param thread_start_t start_fn holds the pointer address of thread start
/// \param stack_top      stack_top holds stack pointer of the thread
/// \return               0 if successful
int thread_create(struct thread *thread, thread_start_t start_fn, void *stack_top)
{
	task_state_init(&thread->task_state, (void *) start_fn, stack_top);

	return 0;
}
