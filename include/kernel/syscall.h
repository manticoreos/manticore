#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

enum {
	SYS_exit	= 1,
};

int syscall(int nr, ...);

#endif
