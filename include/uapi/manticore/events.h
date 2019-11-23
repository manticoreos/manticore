#ifndef __MANTICORE_UAPI_EVENTS_H
#define __MANTICORE_UAPI_EVENTS_H

#include <stddef.h>

enum {
	EVENT_PACKET_RX = 0x01,
};

struct event {
	size_t type;
	void *addr;
	size_t len;
};

#endif
