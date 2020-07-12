#include <sys/epoll.h>

#include <errno.h>

#include <manticore/atomic-ring-buffer.h>
#include <manticore/events.h>
#include <manticore/syscalls.h>

#include "internal/net.h"

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

static struct epoll_event interest_set[1]; // FIXME: make bigger

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	if (epfd != EPOLL_FD) {
		errno = EBADF;
		return -1;
	}
	interest_set[0] = *event;
	return 0;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int err;

	if (epfd != EPOLL_FD) {
		errno = EBADF;
		return -1;
	}
	if (maxevents <= 0) {
		errno = EINVAL;
		return -1;
	}

	err = wait();
	if (err) {
		errno = -err;
		return -1;
	}

	struct atomic_ring_buffer *queue;
	err = getevents((void **)&queue);
	if (err) {
		errno = -err;
		return -1;
	}

	int nr_events = 0;
	while (!atomic_ring_buffer_is_empty(queue) && nr_events < maxevents) {
		struct event *kern_event = atomic_ring_buffer_front(queue);
		uint32_t ep_events = 0;

		switch (kern_event->type) {
		case EVENT_PACKET_RX: {
			struct packet_view pk = {
				.start = kern_event->addr,
				.end = kern_event->addr + kern_event->len,
			};
			if (net_input(&pk)) {
				struct epoll_event *ep_event = &events[nr_events++];
				*ep_event = interest_set[0]; // FIXME
				ep_event->events = EPOLLIN;
			}
			break;
		}
		default:
			break;
		}
		atomic_ring_buffer_pop(queue);
	}
	return nr_events;
}
