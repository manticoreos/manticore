#include <manticore/io_queue.h>

#include <manticore/atomic-ring-buffer.h>
#include <manticore/io_queue_abi.h>
#include <manticore/syscalls.h>

static int __io_queue_append(io_queue_t queue, enum io_opcode opcode, void *addr, size_t len)
{
	struct atomic_ring_buffer *buf = queue;
	struct io_cmd io_cmd = {
	    .opcode = opcode,
	    .addr = addr,
	    .len = len,
	};
	atomic_ring_buffer_emplace(buf, &io_cmd, sizeof(struct io_cmd));

	return 0;
}

int io_submit(io_queue_t queue, void *addr, size_t len)
{
	return __io_queue_append(queue, IO_OPCODE_SUBMIT, addr, len);
}

int io_complete(io_queue_t queue, void *addr, size_t len)
{
	return __io_queue_append(queue, IO_OPCODE_COMPLETE, addr, len);
}
