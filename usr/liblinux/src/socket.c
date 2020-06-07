#include <sys/socket.h>

#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <sys/types.h>

#include "internal/socket.h"

int accept(int sockfd, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_accept(sk, addr, addrlen);
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_bind(sk, addr, addrlen);
}

int getsockopt(int sockfd, int level, int optname, void *restrict optval, socklen_t *restrict optlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_getsockopt(sk, level, optname, optval, optlen);
}

int listen(int sockfd, int backlog)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_listen(sk, backlog);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
	return recvfrom(sockfd, buf, len, flags, NULL, NULL);
}

ssize_t recvfrom(int sockfd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_recvfrom(sk, buf, len, flags, src_addr, addrlen);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
	return sendto(sockfd, buf, len, flags, NULL, 0);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_sendto(sk, buf, len, flags, dest_addr, addrlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_setsockopt(sk, level, optname, optval, optlen);
}

int shutdown(int sockfd, int how)
{
	struct socket *sk = socket_lookup_by_fd(sockfd);
	if (!sk) {
		errno = EBADF;
		return -1;
	}
	return socket_shutdown(sk, how);
}

int socket(int domain, int type, int protocol)
{
	return socket_alloc(domain, type, protocol);
}
