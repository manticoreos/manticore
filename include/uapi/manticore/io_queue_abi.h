#ifndef __MANTICORE_UAPI_IO_QUEUE_H
#define __MANTICORE_UAPI_IO_QUEUE_H

#include <stddef.h>

typedef void *io_queue_t;

struct io_cmd {
	void *addr;
	size_t len;
};

#endif
