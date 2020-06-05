#include "internal/net.h"

#include "internal/setup.h"
#include "internal/trace.h"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include <string.h>
#include <stdio.h>

struct net_statistics {
	uint64_t	packets_dropped;
};

static struct net_statistics stats;

static bool net_input_one(struct packet_view *pk)
{
	LIBLINUX_TRACE(net_input);

	struct ethhdr *ethh = pk->start;

	if (packet_view_len(pk) < sizeof(*ethh)) {
		stats.packets_dropped++;
		packet_view_trim(pk, packet_view_len(pk));
		return false;
	}
	packet_view_trim(pk, sizeof(*ethh));

	if (ethh->h_proto == ntohs(ETH_P_IP)) {
		ip_input(pk);
		return true;
	} else if (ethh->h_proto == ntohs(ETH_P_ARP)) {
		arp_input(pk);
		return false;
	} else {
		stats.packets_dropped++;
		packet_view_trim(pk, packet_view_len(pk));
		return false;
	}
}

bool net_input(struct packet_view *pk)
{
	bool ret = false;

	while (packet_view_len(pk) > 0) {
		ret |= net_input_one(pk);
	}

	return ret;
}
