#include <kernel/syscall.h>

#include <kernel/errno.h>
#include <kernel/panic.h>

#include <stdarg.h>

static int sys_exit(int status)
{
	panic("Process terminated.");
	return 0;
}

#define SYSCALL1(fn, arg0_type)                                                                                        \
	case (SYS_##fn):                                                                                               \
		do {                                                                                                   \
			va_list args;                                                                                  \
			arg0_type arg0;                                                                                \
			va_start(args, nr);                                                                            \
			arg0 = va_arg(args, arg0_type);                                                                \
			va_end(args);                                                                                  \
			return sys_##fn(arg0);                                                                         \
		} while (0)

int syscall(int nr, ...)
{
	switch (nr) {
		SYSCALL1(exit, int);
	}
	return -ENOSYS;
}
