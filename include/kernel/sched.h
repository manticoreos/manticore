#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

void schedule();

int process_subscribe(const char *name);
void process_wait(void);
void wake_up_processes(void);

#endif
