#ifndef MANTICORE_SYSCALLS_H
#define MANTICORE_SYSCALLS_H

enum {
	SYS_exit	= 1,
};

void exit(int status) __attribute__ ((noreturn));

long syscall1(long number, long arg0);

#endif
