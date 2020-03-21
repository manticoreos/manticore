#include <sys/socket.h>

#include <netinet/in.h>
#include <sys/types.h>

#include <errno.h>
#include <stddef.h>

#include <manticore/syscalls.h>

#define SOCKET_FD_OFFSET 100

struct socket {
	int domain;
	int type;
	int protocol;
};

#define MAX_SOCKETS 8

static struct socket sockets[MAX_SOCKETS];
static size_t nr_sockets;

static struct socket *lookup_socket(int sockfd)
{
	if (sockfd < SOCKET_FD_OFFSET) {
		return NULL;
	}
	int idx = sockfd - SOCKET_FD_OFFSET;
	if (idx > MAX_SOCKETS) {
		return NULL;
	}
	return &sockets[idx];
}

int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
{
	errno = EBADF;
	return -1;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	struct socket *sock = lookup_socket(sockfd);
	if (!sock) {
		errno = EBADF;
		return -1;
	}
	/* FIXME: Call subscribe() to register to a specific flow.  */
	return 0;
}

int getsockopt(int sockfd, int level, int optname, void *restrict optval, socklen_t *restrict optlen)
{
	errno = EBADF;
	return -1;
}

int listen(int sockfd, int backlog)
{
	errno = EBADF;
	return -1;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
	return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

ssize_t recvfrom(int sockfd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict src_addr,
		 socklen_t *restrict addrlen)
{
	errno = EBADF;
	return -1;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	return sendto(sockfd, buf, len, flags, NULL, 0);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	errno = EBADF;
	return -1;
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	errno = EBADF;
	return -1;
}

int shutdown(int socket, int how)
{
	errno = EBADF;
	return -1;
}

int socket(int domain, int type, int protocol)
{
	if (domain != AF_INET || type != SOCK_DGRAM || protocol != 0) {
		errno = EINVAL;
		return -1;
	}
	if (nr_sockets >= MAX_SOCKETS) {
		errno = EMFILE;
		return -1;
	}
	int sockfd = SOCKET_FD_OFFSET + nr_sockets++;

	struct socket *sock = &sockets[sockfd];
	sock->domain = domain;
	sock->type = type;
	sock->protocol = protocol;

	return sockfd;
}
