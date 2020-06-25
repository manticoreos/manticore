#ifndef __LIBLINUX_INTERNAL_SOCKET_H
#define __LIBLINUX_INTERNAL_SOCKET_H

#include <stddef.h>
#include <stdint.h>

#include <sys/socket.h>

struct packet_view;
struct socket;

struct socket_operations {
	ssize_t (*recvfrom)(struct socket *sk, void *restrict buf, size_t len, int flags,
			    struct sockaddr *restrict src_addr, socklen_t *restrict addrlen);
	ssize_t (*sendto)(struct socket *sk, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
			  socklen_t addrlen);
};

struct socket {
	const struct socket_operations *ops;
	uint16_t local_port;
	char rx_buffer[1500]; /* FIXME make bigger */
};

int socket_alloc(int domain, int type, int protocol);

struct socket *socket_lookup_by_fd(int sockfd);
struct socket *socket_lookup_by_flow(uint16_t local_port, uint16_t foreign_port);

void socket_input(struct socket *sk, struct packet_view *pk);

int socket_accept(struct socket *sk, struct sockaddr *restrict addr, socklen_t *restrict addrlen);
int socket_bind(struct socket *sk, const struct sockaddr *addr, socklen_t addrlen);
int socket_getsockopt(struct socket *sk, int level, int optname, void *restrict optval, socklen_t *restrict optlen);
int socket_listen(struct socket *sk, int backlog);
ssize_t socket_recvfrom(struct socket *sk, void *restrict buf, size_t len, int flags,
			struct sockaddr *restrict src_addr, socklen_t *restrict addrlen);
ssize_t socket_sendto(struct socket *sk, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
		      socklen_t addrlen);
int socket_setsockopt(struct socket *sk, int level, int optname, const void *optval, socklen_t optlen);
int socket_shutdown(struct socket *sk, int how);

#endif
