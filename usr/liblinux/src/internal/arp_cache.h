#ifndef __LIBLINUX_INTERNAL_ARP_CACHE_H
#define __LIBLINUX_INTERNAL_ARP_CACHE_H

#include <linux/if_ether.h> /* for ETH_ALEN */
#include <netinet/in.h>     /* for in_addr_t */

struct arp_cache_entry {
	in_addr_t ip_addr;
	uint8_t mac_addr[ETH_ALEN];
};

#define ARP_CACHE_SHIFT 7
#define ARP_CACHE_SIZE (1 << ARP_CACHE_SHIFT)

struct arp_cache {
	struct arp_cache_entry entries[ARP_CACHE_SIZE];
};

void arp_cache_init(struct arp_cache *cache);

void arp_cache_insert(struct arp_cache *cache, in_addr_t ip_addr, uint8_t *mac_addr);

uint8_t *arp_cache_lookup(struct arp_cache *cache, in_addr_t ip_addr);

#endif
