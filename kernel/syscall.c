#include <kernel/syscall.h>

#include <kernel/errno.h>
#include <kernel/panic.h>
#include <kernel/sched.h>

#include <stdarg.h>

static int sys_exit(int status)
{
	panic("Process terminated with exit status %d.", status);
	return 0;
}

static int sys_wait(void)
{
	process_wait();

	return 0;
}

static int sys_console_print(const char *text)
{
	/* FIXME: This is unsafe -- we are following a pointer from
	   userspace without verifying it.  */
	puts(text);
	return 0;
}

#define SYSCALL0(fn)                                                                                                   \
	case (SYS_##fn):                                                                                               \
		do {                                                                                                   \
			return sys_##fn();                                                                             \
		} while (0)

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
	SYSCALL0(wait);
	SYSCALL1(console_print, const char *);
	}
	return -ENOSYS;
}
