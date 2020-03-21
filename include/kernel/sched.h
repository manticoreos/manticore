#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

void schedule();

int process_subscribe(const char *name);
void *process_getevents(void);
int process_get_config(int desc, int opt, void *buf, size_t len);
void *process_get_io_queue(void);
int process_acquire(const char *name, int flags);
void process_wait(void);
void wake_up_processes(void);

#endif
