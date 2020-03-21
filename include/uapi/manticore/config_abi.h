#ifndef __MANTICORE_UAPI_CONFIG_ABI_H
#define __MANTICORE_UAPI_CONFIG_ABI_H

/*
 * Ethernet device configuration options:
 */
enum {
	/* The MAC address of an ethernet interface.  */
	CONFIG_ETHERNET_MAC_ADDRESS = 0,
	/* The I/O queue of the ethernet interface.  */
	CONFIG_IO_QUEUE = 1,
};

#endif
