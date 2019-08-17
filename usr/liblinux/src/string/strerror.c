#include <string.h>

#include <errno.h>

char *strerror(int errnum)
{
	switch (errnum) {
	case EBADF:
		return "Bad file descriptor";
	case EINVAL:
		return "Invalid argument";
	case EMFILE:
		return "Too many open files";
	case ENOSYS:
		return "Invalid system call number";
	default:
		return "Unknown error";
	}
}
