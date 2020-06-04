#ifndef __LIBLINUX_INTERNAL_NET_H
#define __LIBLINUX_INTERNAL_NET_H

#include <stdbool.h>
#include <stddef.h>

/// A packet buffer (pbuf) descriptor.
///
/// A packet buffer descriptor specifies an (start, end) tuple that points to a
/// contiguous memory area that contains one packet.
struct pbuf {
	void *start;
	void *end;
};

/// Trims \size bytes from the packet buffer \pbuf.
static inline void pbuf_trim_front(struct pbuf *pbuf, size_t size)
{
	pbuf->start += size;
}

/// Returns the length of the packet pointed to by \pbuf
static inline size_t pbuf_len(struct pbuf *pbuf)
{
	return pbuf->end - pbuf->start;
}

/// Forward a packet buffer to the network stack.
///
/// \return @true if packet caused an epoll event; otherwise returns @false.
bool net_input(struct pbuf *pbuf);
void arp_input(struct pbuf *pbuf);

#endif
