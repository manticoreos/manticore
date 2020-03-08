#include <manticore/io_queue.h>

#include <manticore/io_queue_abi.h>
#include <manticore/atomic-ring-buffer.h>
#include <manticore/syscalls.h>

int io_submit(io_queue_t queue, void *addr, size_t len)
{
	struct atomic_ring_buffer *buf = queue;
	struct io_cmd io_cmd = {
		.addr = addr,
		.len = len,
	};
	atomic_ring_buffer_emplace(buf, &io_cmd, sizeof(struct io_cmd));

	return 0;
}
