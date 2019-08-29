#ifndef __SYS_EPOLL_H
#define __SYS_EPOLL_H

#include <stdint.h>

#define EPOLL_CTL_ADD 1
#define EPOLL_CT_DEL 2
#define EPOLL_CTL_MOD 3

enum epoll_events {
	EPOLLIN = 0x0001,
	EPOLLHUP = 0x0010,
};

typedef union epoll_data {
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event {
	uint32_t events;
	epoll_data_t data;
} __attribute__((__packed__));

int epoll_create(int size);
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif
