#ifndef MANTICORE_SYSCALLS_H
#define MANTICORE_SYSCALLS_H

enum {
	SYS_exit	= 1,
	SYS_wait	= 2,
};

void exit(int status) __attribute__ ((noreturn));
void wait(void);

long syscall0(long number);
long syscall1(long number, long arg0);

#endif
