#include "internal/setup.h"

#include <manticore/config_abi.h>
#include <manticore/syscalls.h>

#include <stdio.h>

char __liblinux_mac_addr[ETH_ALEN];

void __liblinux_setup(void)
{
	get_config("/dev/eth", CONFIG_ETHERNET_MAC_ADDRESS, __liblinux_mac_addr, ETH_ALEN);

	fprintf(stderr, "liblinux: MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", __liblinux_mac_addr[0],
		__liblinux_mac_addr[1], __liblinux_mac_addr[2], __liblinux_mac_addr[3], __liblinux_mac_addr[4],
		__liblinux_mac_addr[5]);
}
