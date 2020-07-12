//
// An atomic ring buffer.
//
// This is a concurrent ring buffer that the kernel and a user space process
// use to communicate with each other over shared memory. The implementation is
// a single-producer (SPSC) queue that is bounded, lock-free, and wait-free.
//

#include <kernel/atomic-ring-buffer.h>

#include <kernel/align.h>

#include <stdatomic.h>
#include <stddef.h>
#include <string.h>

struct atomic_ring_buffer *atomic_ring_buffer_new(void *buf, size_t buf_size, size_t element_size)
{
	struct atomic_ring_buffer *ret = buf;
	memset(ret, 0, sizeof(struct atomic_ring_buffer));
	ret->element_size = element_size;

	size_t capacity = buf_size - sizeof(struct atomic_ring_buffer);
	ret->capacity = capacity - (capacity % element_size);

	return ret;
}

bool atomic_ring_buffer_is_empty(struct atomic_ring_buffer *queue)
{
	uint64_t head = atomic_load_explicit(&queue->head, memory_order_acquire);
	uint64_t tail = atomic_load_explicit(&queue->tail, memory_order_acquire);
	return head == tail;
}

void *atomic_ring_buffer_front(struct atomic_ring_buffer *queue)
{
	uint64_t head = atomic_load_explicit(&queue->head, memory_order_acquire);
	uint64_t tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
	if (head == tail) {
		return NULL;
	}
	return (void *)queue->data + tail;
}

void atomic_ring_buffer_pop(struct atomic_ring_buffer *queue)
{
	unsigned long tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
	unsigned long next_tail = tail + queue->element_size;
	if (next_tail == queue->capacity) {
		next_tail = 0;
	}
	atomic_store_explicit(&queue->tail, next_tail, memory_order_release);
}

bool atomic_ring_buffer_emplace(struct atomic_ring_buffer *queue, void *element)
{
	uint64_t head = atomic_load_explicit(&queue->head, memory_order_relaxed);
	uint64_t next_head = head + queue->element_size;
	if (next_head == queue->capacity) {
		next_head = 0;
	}
	if (next_head == atomic_load_explicit(&queue->tail, memory_order_acquire)) {
		return false;
	}
	memcpy(queue->data + head, element, queue->element_size);
	atomic_store_explicit(&queue->head, next_head, memory_order_release);
	return true;
}
