#include <manticore/atomic-ring-buffer.h>

#include <stdatomic.h>
#include <stddef.h>
#include <string.h>

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

void atomic_ring_buffer_pop(struct atomic_ring_buffer *queue, size_t element_size)
{
	unsigned long tail = atomic_load_explicit(&queue->tail, memory_order_relaxed);
	unsigned long next_tail = tail + element_size;
	if (next_tail == queue->capacity) {
		next_tail = 0;
	}
	atomic_store_explicit(&queue->tail, next_tail, memory_order_release);
}

/* The libmanticore library has no dependency to libc, so we need to implement
   memcpy() ourselves.  Please consider this as an official apology to whoever
   discovers this to be a performance bottleneck for kernel and user space
   communication.  */
static void *memcpy_internal(void *dest, const void *src, size_t n)
{
       char *d = dest;
       const char *s = src;
       for (int i = 0; i < n; i++) {
               *d++ = *s++;
       }
}

bool atomic_ring_buffer_emplace(struct atomic_ring_buffer *queue, void *element, size_t element_size)
{
	uint64_t head = atomic_load_explicit(&queue->head, memory_order_relaxed);
	uint64_t next_head = head + element_size;
	if (next_head == queue->capacity) {
		next_head = 0;
	}
	if (next_head == atomic_load_explicit(&queue->tail, memory_order_acquire)) {
		return false;
	}
	memcpy_internal(queue->data + head, element, element_size);
	atomic_store_explicit(&queue->head, next_head, memory_order_release);
	return true;
}
