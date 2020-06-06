#include "internal/net.h"

#include "internal/setup.h"

#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include <string.h>
#include <stdio.h>

struct net_statistics {
	uint64_t	packets_dropped;
};

static struct net_statistics stats;

static bool net_input_one(struct pbuf *pbuf)
{
	struct ethhdr *ethh = pbuf->start;

	if (pbuf_len(pbuf) < sizeof(*ethh)) {
		stats.packets_dropped++;
		pbuf_trim_front(pbuf, pbuf_len(pbuf));
		return false;
	}
	pbuf_trim_front(pbuf, sizeof(*ethh));

	if (ethh->h_proto == ntohs(ETH_P_ARP)) {
		arp_input(pbuf);
		return false;
	} else {
		stats.packets_dropped++;
		pbuf_trim_front(pbuf, pbuf_len(pbuf));
		return false;
	}
}

bool net_input(struct pbuf *pbuf)
{
	bool ret = false;

	while (pbuf_len(pbuf) > 0) {
		ret |= net_input_one(pbuf);
	}

	return ret;
}