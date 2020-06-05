#include "internal/net.h"

#include "internal/setup.h"
#include "internal/socket.h"
#include "internal/trace.h"

#include <arpa/inet.h>
#include <assert.h> // FIXME
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define WARN(str) fprintf(stderr, "warning: %s\n", str)

ssize_t udp_recvfrom(struct socket *sk, void *restrict buf, size_t len, int flags, struct sockaddr *restrict src_addr,
		     socklen_t *restrict addrlen)
{
	LIBLINUX_TRACE(udp_recvfrom);

	struct iphdr *iph = sk->rx_buffer;

	// FIXME: check pbuf length

	struct udphdr *udph = sk->rx_buffer + sizeof(struct iphdr);

	uint16_t udp_len = ntohs(udph->len);

	size_t data_off = sizeof(struct iphdr) + sizeof(struct udphdr);
	size_t data_len = udp_len - sizeof(struct udphdr);

	// FIXME: check pbuf length

	if (*addrlen < sizeof(struct sockaddr_in)) {
		errno = EINVAL;
		return -1;
	}
	struct sockaddr_in *saddr_in = (void *)src_addr;
	saddr_in->sin_addr.s_addr = iph->saddr;
	saddr_in->sin_port = udph->source;
	*addrlen = sizeof(struct sockaddr_in);

	// FIXME: How can we retrieve rest of the data?
	size_t nr = data_len;
	if (nr > len) {
		nr = len;
	}
	memcpy(buf, sk->rx_buffer + data_off, nr);

	return nr;
}

static inline uint64_t checksum_add(uint64_t sum, const void *data, size_t count)
{
	const uint32_t *p = data;
	while (count >= 4) {
		sum += *p++;
		count -= 4;
	}
	const uint16_t *q = (void *)p;
	if (count >= 2) {
		sum += *q++;
		count -= 2;
	}
	const uint8_t *r = (void *)q;
	if (count) {
		sum += *r++;
		count -= 1;
	}
	return sum;
}

static inline uint16_t checksum_finalize(uint64_t sum)
{
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return ~sum;
}

uint16_t ipv4_checksum(const void *buf, size_t len)
{
	uint64_t sum = checksum_add(0, buf, len);

	return checksum_finalize(sum);
}

uint16_t udp_checksum(const void *buf, size_t len, in_addr_t dest_ip, in_addr_t src_ip)
{
	uint64_t sum = checksum_add(0, buf, len);

	// Add IPv4 "pseudo-header" to the sum:
	sum += src_ip & 0xffffU;
	sum += src_ip >> 16;

	sum += dest_ip & 0xffffU;
	sum += dest_ip >> 16;

	sum += htons(IPPROTO_UDP);
	sum += htons(len);

	return checksum_finalize(sum);
}

struct packet_buf {
	void *buf;
	size_t len;
	size_t capacity;
};

void packet_buf_init(struct packet_buf *pk, void *buf, size_t capacity)
{
	pk->buf = buf;
	pk->len = 0;
	pk->capacity = capacity;
}

void *packet_buf_start(struct packet_buf *pk)
{
	return pk->buf;
}

size_t packet_buf_len(struct packet_buf *pk)
{
	return pk->len;
}

void *packet_buf_reserve(struct packet_buf *pk, size_t size)
{
	assert(pk->len + size < pk->capacity);
	size_t off = pk->len;
	pk->len += size;
	return pk->buf + off;
}

void packet_buf_append(struct packet_buf *pk, const void *buf, size_t len)
{
	void *dst = packet_buf_reserve(pk, len);

	memcpy(dst, buf, len);
}

struct ethhdr *ethhdr_append(struct packet_buf *pk, const char *dest, const char *source, uint16_t proto)
{
	struct ethhdr *ethh = packet_buf_reserve(pk, sizeof(*ethh));
	if (ethh) {
		memcpy(ethh->h_dest, dest, ETH_ALEN);
		memcpy(ethh->h_source, source, ETH_ALEN);
		ethh->h_proto = htons(proto);
	}
	return ethh;
}

struct iphdr *iphdr_append(struct packet_buf *pk, uint16_t tot_len, uint8_t proto, in_addr_t dest_ip, in_addr_t src_ip)
{
	struct iphdr *iph = packet_buf_reserve(pk, sizeof(*iph));
	if (iph) {
		iph->version = 4;
		iph->ihl = 5;
		iph->tos = 0;
		iph->tot_len = htons(tot_len);
		iph->id = 0;
		iph->frag_off = 0;
		iph->ttl = 64;
		iph->protocol = proto;
		iph->check = 0;
		iph->saddr = src_ip;
		iph->daddr = dest_ip;
	}
	return iph;
}

struct udphdr *udphdr_append(struct packet_buf *pk, uint16_t len, in_port_t dest, in_port_t source)
{
	struct udphdr *udph = packet_buf_reserve(pk, sizeof(*udph));
	if (udph) {
		udph->source = source;
		udph->dest = dest;
		udph->len = htons(len);
		udph->check = 0;
	}
	return udph;
}

ssize_t udp_sendto(struct socket *sk, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
		   socklen_t addrlen)
{
	LIBLINUX_TRACE(udp_sendto);

	if (addrlen != sizeof(struct sockaddr_in)) {
		errno = EINVAL;
		return -1;
	}

	struct sockaddr_in *saddr_in = (void *)dest_addr;

	in_addr_t src_ip = __liblinux_host_ip;
	in_addr_t dest_ip = saddr_in->sin_addr.s_addr;

	char *dest_arp = arp_cache_lookup(&__liblinux_arp_cache, dest_ip);
	assert(dest_arp != NULL);
	char *src_arp = __liblinux_mac_addr;

	uint16_t udp_len = sizeof(struct udphdr) + len;
	uint16_t ip_len = sizeof(struct iphdr) + udp_len;

	static char _tx_buf[4096]; /* FIXME */

	struct packet_buf pk;
	packet_buf_init(&pk, _tx_buf, 4096);

	ethhdr_append(&pk, dest_arp, src_arp, ETH_P_IP);

	struct iphdr *iph = iphdr_append(&pk, ip_len, IPPROTO_UDP, dest_ip, src_ip);

	struct udphdr *udph = udphdr_append(&pk, udp_len, saddr_in->sin_port, sk->local_port);

	packet_buf_append(&pk, buf, len);

	iph->check = ipv4_checksum(iph, sizeof(*iph));

	udph->check = udp_checksum(udph, udp_len, dest_ip, src_ip);

	io_submit(__liblinux_eth_ioqueue, packet_buf_start(&pk), packet_buf_len(&pk)); /* FIXME: error handling */

	return len;
}

static void udp_input(struct packet_view *pk)
{
	LIBLINUX_TRACE(udp_input);

	const struct udphdr *udph = pk->start + sizeof(struct iphdr);

	uint16_t udp_len = ntohs(udph->len);

	struct socket *sk = socket_lookup_by_flow(udph->dest, udph->source);

	assert(sk != NULL);

	socket_input(sk, pk);

	packet_view_trim(pk, sizeof(struct iphdr) + udp_len);
}

void ip_input(struct packet_view *pk)
{
	LIBLINUX_TRACE(ip_input);


	const struct iphdr *iph = pk->start;

	if (packet_view_len(pk) < sizeof(*iph)) {
		return;
	}

	uint16_t ip_len = ntohs(iph->tot_len);
	if (ip_len < sizeof(*iph)) {
		return;
	}
	if (iph->version != 4) {
		return;
	}
	/* FIXME: verify checksum */

	/* FIXME: fragmentation */

	switch (iph->protocol) {
	case IPPROTO_UDP:
		udp_input(pk);
		break;
	default:
		WARN("dropping packet, unknown header");
		break;
	}
}
