#include "internal/setup.h"

#include <manticore/config_abi.h>
#include <manticore/syscalls.h>

#include <assert.h>
#include <linux/if_ether.h>
#include <stdio.h>

char __liblinux_mac_addr[ETH_ALEN];

void __liblinux_setup(void)
{
	int eth_desc = acquire("/dev/eth", 0);
	if (eth_desc < 0) {
		assert(0);
	}
	get_config(eth_desc, CONFIG_ETHERNET_MAC_ADDRESS, __liblinux_mac_addr, ETH_ALEN);
	fprintf(stderr, "MAC address = %02x:%02x:%02x:%02x:%02x:%02x\n", __liblinux_mac_addr[0], __liblinux_mac_addr[1],
		__liblinux_mac_addr[2], __liblinux_mac_addr[3], __liblinux_mac_addr[4], __liblinux_mac_addr[5]);
}
