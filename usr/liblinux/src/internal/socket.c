/*
 * User-level socket interface.
 */

#include "internal/socket.h"

#include "internal/net.h"

#include <errno.h>
#include <netinet/in.h>
#include <string.h>

static const struct socket_operations udp_socket_ops = {
    .recvfrom = udp_recvfrom,
    .sendto = udp_sendto,
};

/* Maximum number of supported sockets.  */
#define MAX_SOCKETS 8
static struct socket sockets[MAX_SOCKETS];
static size_t nr_sockets;

/* The starting offset of socket file descriptors.  */
#define SOCKET_FD_OFFSET 100

static struct socket_operations *find_socket_ops(int domain, int type, int protocol)
{
	if (domain != AF_INET || type != SOCK_DGRAM || protocol != 0) {
		return NULL;
	}
	return &udp_socket_ops;
}

int socket_alloc(int domain, int type, int protocol)
{
	if (nr_sockets >= MAX_SOCKETS) {
		errno = EMFILE;
		return -1;
	}
	struct socket_operations *ops = find_socket_ops(domain, type, protocol);
	if (!ops) {
		errno = EINVAL;
		return -1;
	}
	int sockidx = nr_sockets++;
	int sockfd = SOCKET_FD_OFFSET + sockidx;

	struct socket *sk = &sockets[sockidx];
	sk->ops = ops;
	sk->local_port = 0;

	return sockfd;
}

struct socket *socket_lookup_by_fd(int sockfd)
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

struct socket *socket_lookup_by_flow(uint16_t local_port, uint16_t foreign_port)
{
	for (int i = 0; i < MAX_SOCKETS; i++) {
		// FIXME: Optimize lookup.
		struct socket *sk = &sockets[i];
		if (sk->local_port == local_port) {
			return sk;
		}
	}
	return NULL;
}

void socket_input(struct socket *sk, struct packet_view *pk)
{
	// FIXME: This overwrites existing data.
	memcpy(sk->rx_buffer, pk->start, packet_view_len(pk));
}

int socket_accept(struct socket *sk, struct sockaddr *restrict addr, socklen_t *restrict addrlen)
{
	errno = EINVAL;
	return -1;
}

int socket_bind(struct socket *sk, const struct sockaddr *addr, socklen_t addrlen)
{
	if (addrlen != sizeof(struct sockaddr_in)) {
		errno = EINVAL;
		return -1;
	}

	struct sockaddr_in *sa_in = (void *)addr;

	sk->local_port = sa_in->sin_port;

	/* FIXME: Tell the kernel we are interested in this flow. */
	return 0;
}

int socket_getsockopt(struct socket *sk, int level, int optname, void *restrict optval, socklen_t *restrict optlen)
{
	errno = ENOPROTOOPT;
	return -1;
}

int socket_listen(struct socket *sk, int backlog)
{
	errno = EOPNOTSUPP;
	return -1;
}

ssize_t socket_recvfrom(struct socket *sk, void *restrict buf, size_t len, int flags,
			struct sockaddr *restrict src_addr, socklen_t *restrict addrlen)
{
	return sk->ops->recvfrom(sk, buf, len, flags, src_addr, addrlen);
}

ssize_t socket_sendto(struct socket *sk, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
		      socklen_t addrlen)
{
	return sk->ops->sendto(sk, buf, len, flags, dest_addr, addrlen);
}

int socket_setsockopt(struct socket *sk, int level, int optname, const void *optval, socklen_t optlen)
{
	errno = ENOPROTOOPT;
	return -1;
}

int socket_shutdown(struct socket *sk, int how)
{
	return 0;
}
