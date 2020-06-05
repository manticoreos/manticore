#include "internal/net.h"

#include "internal/arp_cache.h"
#include "internal/setup.h"
#include "internal/trace.h"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <manticore/config_abi.h>
#include <manticore/io_queue.h>
#include <manticore/syscalls.h>

#include <string.h>

struct arp_ip4 {
	/// MAC address of the sender of this ARP packet.
	uint8_t ar_smac[ETH_ALEN];
	/// IP address of the sender of this ARP packet.
	uint32_t ar_sip;
	/// MAC address of the sender of this ARP packet.
	unsigned char ar_dmac[ETH_ALEN];
	uint32_t ar_dip;
} __attribute__((packed));

static void arp_reply(struct arphdr *request_arph)
{
	LIBLINUX_TRACE(arp_reply);

	// FIXME: Use some generic TX buffer region
	static char tx_buf[128];

	struct arp_ip4 *request_arp = (void *)request_arph + sizeof(*request_arph);
	struct ethhdr *reply_ethh = (void *) tx_buf;
	struct arphdr *reply_arph = (void *) tx_buf + sizeof(*reply_ethh);
	struct arp_ip4 *reply_arp = (void *) tx_buf + sizeof(*reply_ethh) + sizeof(*reply_arph);

	arp_cache_insert(&__liblinux_arp_cache, request_arp->ar_sip, request_arp->ar_smac);

	memcpy(reply_ethh->h_dest, request_arp->ar_smac, ETH_ALEN);
	memcpy(reply_ethh->h_source, __liblinux_mac_addr, ETH_ALEN);
	reply_ethh->h_proto = htons(ETH_P_ARP);
	reply_arph->ar_hrd = htons(ARPHRD_ETHER);
	reply_arph->ar_pro = htons(ETH_P_IP);
	reply_arph->ar_hln = ETH_ALEN;
	reply_arph->ar_pln = 4;
	reply_arph->ar_op = htons(ARPOP_REPLY);

	memcpy(reply_arp->ar_smac, __liblinux_mac_addr, ETH_ALEN);
	reply_arp->ar_sip = __liblinux_host_ip;

	memcpy(reply_arp->ar_dmac, request_arp->ar_smac, ETH_ALEN);
	reply_arp->ar_dip = request_arp->ar_sip;

	// FIXME: io_submit() error handling
	io_submit(__liblinux_eth_ioqueue, reply_ethh, sizeof(*reply_ethh) + sizeof(*reply_arph) + sizeof(*reply_arp));
}

void arp_input(struct packet_view *pk)
{
	LIBLINUX_TRACE(arp_input);

	struct arphdr *arph = pk->start;

	switch (ntohs(arph->ar_op)) {
	case ARPOP_REQUEST:
		arp_reply(arph);
		break;
	default:
		break;
	}

	// FIXME: Notify OS that packet is consumed.
}
