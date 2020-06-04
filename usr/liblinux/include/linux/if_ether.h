#ifndef __LINUX_IF_ETHER_H
#define __LINUX_IF_ETHER_H

#include <stdint.h>

#define ETH_ALEN 6

#define ETH_P_IP 0x0800
#define ETH_P_ARP 0x0806

struct ethhdr {
	uint8_t h_dest[ETH_ALEN];
	uint8_t h_source[ETH_ALEN];
	uint16_t h_proto;
} __attribute__((packed));

#endif
