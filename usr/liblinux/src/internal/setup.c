#include "internal/setup.h"

#include "internal/arp_cache.h"

#include <manticore/config_abi.h>
#include <manticore/syscalls.h>

#include <assert.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <stdio.h>

char __liblinux_mac_addr[ETH_ALEN];

io_queue_t __liblinux_eth_ioqueue;

// FIXME: This is the default QEMU SLIPR guest IP address. Make it configurable.
#define HOST_IP_ADDR "10.0.2.15"

uint32_t __liblinux_host_ip;

struct arp_cache __liblinux_arp_cache;

extern void __liblinux_malloc_init(void);

void __liblinux_setup(void)
{
	int eth_desc = acquire("/dev/eth", 0);
	if (eth_desc < 0) {
		assert(0);
	}

	get_config(eth_desc, CONFIG_IO_QUEUE, &__liblinux_eth_ioqueue, sizeof(io_queue_t));

	get_config(eth_desc, CONFIG_ETHERNET_MAC_ADDRESS, __liblinux_mac_addr, ETH_ALEN);

	fprintf(stderr, "MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __liblinux_mac_addr[0], __liblinux_mac_addr[1],
		__liblinux_mac_addr[2], __liblinux_mac_addr[3], __liblinux_mac_addr[4], __liblinux_mac_addr[5]);

	__liblinux_host_ip = inet_addr(HOST_IP_ADDR);

	fprintf(stderr, "IP address = %s\n", HOST_IP_ADDR);

	arp_cache_init(&__liblinux_arp_cache);

	__liblinux_malloc_init();
}
