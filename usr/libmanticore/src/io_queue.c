#include <manticore/io_queue.h>

#include <manticore/io_queue_abi.h>
#include <manticore/atomic-ring-buffer.h>
#include <manticore/syscalls.h>

int io_submit(void *addr, size_t len)
{
	struct atomic_ring_buffer *queue;
	int err;

	err = get_io_queue((void **) &queue); /* FIXME: optimize the queue lookup */
	if (err) {
		return err;
	}

	struct io_cmd io_cmd = {
		.addr = addr,
		.len = len,
	};
	atomic_ring_buffer_emplace(queue, &io_cmd, sizeof(struct io_cmd));

	return 0;
}
