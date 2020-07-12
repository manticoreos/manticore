#ifndef __MANTICORE_UAPI_IO_QUEUE_H
#define __MANTICORE_UAPI_IO_QUEUE_H

#include <stddef.h>
#include <stdint.h>

typedef void *io_queue_t;

enum io_opcode {
	IO_OPCODE_SUBMIT = 0x1,
};

struct io_cmd {
	uint32_t opcode;
	void *addr;
	size_t len;
};

#endif
