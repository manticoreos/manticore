#ifndef __LIBLINUX_INTERNAL_NET_H
#define __LIBLINUX_INTERNAL_NET_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>

struct socket;

/// A packet descriptor.
///
/// A packet descriptor specifies an (start, end) tuple that points to a
/// contiguous memory area that contains one packet.
struct packet_view {
	void *start;
	void *end;
};

/// Returns the length of the packet pointed to by \pk
static inline size_t packet_view_len(struct packet_view *pk)
{
	return pk->end - pk->start;
}

/// Trims \size bytes from the packet descriptor \pk.
static inline void packet_view_trim(struct packet_view *pk, size_t size)
{
	assert(packet_view_len(pk) >= size);

	pk->start += size;
}

/// Forward a packet descritpro to the network stack.
///
/// \return @true if packet caused an epoll event; otherwise returns @false.
bool net_input(struct packet_view *pk);

void arp_input(struct packet_view *pk);
bool net_input(struct packet_view *pk);
void ip_input(struct packet_view *pk);
void arp_input(struct packet_view *pk);

ssize_t udp_recvfrom(struct socket *sk, void *restrict buf, size_t len, int flags, struct sockaddr *restrict src_addr, socklen_t *restrict addrlen);
ssize_t udp_sendto(struct socket *sk, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

#endif
