#include <sys/epoll.h>

#include <errno.h>
#include <manticore/syscalls.h>

#define EPOLL_FD	200

static int nr_epoll_fds = 0;

static int do_epoll_create(int flags)
{
	if (flags != 0) {
		errno = EINVAL;
		return -1;
	}
	if (nr_epoll_fds++ > 0) {
		errno = EMFILE;
		return -1;
	}
	return EPOLL_FD;
}

int epoll_create(int size)
{
	if (size <= 0) {
		errno = EINVAL;
		return -1;
	}
	return do_epoll_create(0);
}

int epoll_create1(int flags)
{
	return do_epoll_create(flags);
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if (epfd != EPOLL_FD) {
		errno = EBADF;
		return -1;
	}
	// TODO: We need to register event interest set.
	return 0;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	if (epfd != EPOLL_FD) {
		errno = EBADF;
		return -1;
	}
	if (maxevents <= 0) {
		errno = EINVAL;
		return -1;
	}
	wait();
	// TODO: Fill the events array and return number of events.
	return 0;
}
