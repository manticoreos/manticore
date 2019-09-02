#include <kernel/syscall.h>

#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/errno.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/user-access.h>

#include <stdarg.h>
#include <stddef.h>

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

static int sys_subscribe(const char *uevent)
{
#define EVENT_SIZE 32
	char event[EVENT_SIZE];
	int err;

	err = strncpy_from_user(event, uevent, EVENT_SIZE);
	if (err < 0) {
		return err;
	}

	return process_subscribe(event);
}

static ssize_t sys_console_print(const char /* __user */ *ubuf, size_t count)
{
	ssize_t off = 0;

	while (count) {
#define BUF_SIZE 64
		char buf[BUF_SIZE];
		size_t nr;
		int err;

		nr = count;
		if (nr > BUF_SIZE) {
			nr = BUF_SIZE;
		}
		err = memcpy_from_user(buf, ubuf + off, nr);
		if (err < 0) {
			return err;
		}
		console_write(buf, nr);
		count -= nr;
		off += nr;
	}
	return off;
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

#define SYSCALL2(fn, arg0_type, arg1_type)                                                                             \
	case (SYS_##fn):                                                                                               \
		do {                                                                                                   \
			va_list args;                                                                                  \
			arg0_type arg0;                                                                                \
			arg1_type arg1;                                                                                \
			va_start(args, nr);                                                                            \
			arg0 = va_arg(args, arg0_type);                                                                \
			arg1 = va_arg(args, arg1_type);                                                                \
			va_end(args);                                                                                  \
			return sys_##fn(arg0, arg1);                                                                   \
		} while (0)

long syscall(int nr, ...)
{
	switch (nr) {
	SYSCALL1(exit, int);
	SYSCALL0(wait);
	SYSCALL2(console_print, const char *, size_t);
	SYSCALL1(subscribe, const char *);
	}
	return -ENOSYS;
}
