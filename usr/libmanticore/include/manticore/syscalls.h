#ifndef MANTICORE_SYSCALLS_H
#define MANTICORE_SYSCALLS_H

enum {
	SYS_exit		= 1,
	SYS_wait		= 2,
	SYS_console_print	= 3,
};

void exit(int status) __attribute__ ((noreturn));
void wait(void);
void console_print(const char *text);

long syscall0(long number);
long syscall1(long number, long arg0);

#endif
