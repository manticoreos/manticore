#ifndef __LIBLINUX_INTERNAL_SETUP_H
#define __LIBLINUX_INTERNAL_SETUP_H

#include <linux/if_ether.h>

void __liblinux_setup(void);

extern char __liblinux_mac_addr[ETH_ALEN];

#endif
