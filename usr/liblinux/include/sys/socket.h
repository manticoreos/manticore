#ifndef __SYS_SOCKET_H
#define __SYS_SOCKET_H

#include <sys/types.h>

#define SOCK_DGRAM 2

#define MSG_DONTWAIT 0x40

typedef unsigned int socklen_t;

typedef unsigned int sa_family_t;

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

struct sockaddr_storage {
	sa_family_t sa_family;
	char __padding[sizeof(struct sockaddr) - sizeof(sa_family_t)];
};

int accept(int, struct sockaddr *restrict, socklen_t *restrict);

int bind(int, const struct sockaddr *, socklen_t);

int getsockopt(int, int, int, void *restrict, socklen_t *restrict);

int listen(int, int);

ssize_t recv(int, void *, size_t, int);

ssize_t recvfrom(int, void *restrict, size_t, int, struct sockaddr *restrict, socklen_t *restrict);

ssize_t send(int, const void *, size_t, int);

ssize_t sendto(int, const void *, size_t, int, const struct sockaddr *, socklen_t);

int setsockopt(int, int, int, const void *, socklen_t);

int shutdown(int, int);

int socket(int, int, int);

#endif
