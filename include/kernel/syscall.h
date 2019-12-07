#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

enum {
	SYS_exit		= 1,
	SYS_wait		= 2,
	SYS_console_print	= 3,
	SYS_subscribe		= 4,
	SYS_getevents		= 5,
	SYS_get_io_queue	= 6,
};

long syscall(int nr, ...);

#endif
