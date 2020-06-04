#ifndef __LIBLINUX_INTERNAL_SETUP_H
#define __LIBLINUX_INTERNAL_SETUP_H

#include <manticore/io_queue.h>

#include <linux/if_ether.h>

#include "internal/arp_cache.h"

void __liblinux_setup(void);

extern io_queue_t __liblinux_eth_ioqueue;

extern char __liblinux_mac_addr[ETH_ALEN];

extern uint32_t __liblinux_host_ip;

extern struct arp_cache __liblinux_arp_cache;

#endif
