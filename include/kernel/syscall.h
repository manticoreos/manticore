#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

enum {
	SYS_exit	= 1,
	SYS_wait	= 2,
};

int syscall(int nr, ...);

#endif
