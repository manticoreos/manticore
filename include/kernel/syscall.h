#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

enum {
	SYS_exit		= 1,
	SYS_wait		= 2,
	SYS_console_print	= 3,
};

long syscall(int nr, ...);

#endif
