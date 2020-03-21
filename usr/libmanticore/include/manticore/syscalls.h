#ifndef MANTICORE_SYSCALLS_H
#define MANTICORE_SYSCALLS_H

#include <manticore/syscall_abi.h>
#include <manticore/types.h>

#include <stddef.h>

void exit(int status) __attribute__ ((noreturn));
int wait(void);
ssize_t console_print(const char *text, size_t count);
int acquire(const char *name, int flags);
int subscribe(const char *event);
int getevents(void **events);
int get_config(int desc, int opt, void *buf, size_t len);

long syscall0(long number);
long syscall1(long number, long arg0);
long syscall2(long number, long arg0, long arg1);
long syscall4(long number, long arg0, long arg1, long arg2, long arg3);

#endif
