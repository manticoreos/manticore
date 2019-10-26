#ifndef KERNEL_ATOMIC_RING_BUFFER_QUEUE_H
#define KERNEL_ATOMIC_RING_BUFFER_QUEUE_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Alignment for atomic ring buffer metadata elements (e.g. head and tail
// pointers). Basically, aligning to L1 cache line size to avoid false sharing.
#define ATOMIC_RING_BUFFER_ALIGN 64

// An atomic ring buffer.
struct atomic_ring_buffer {
	size_t capacity;
	char capacity_pad[ATOMIC_RING_BUFFER_ALIGN - sizeof(size_t)];
	atomic_ullong tail;
	char tail_pad[ATOMIC_RING_BUFFER_ALIGN - sizeof(atomic_ullong)];
	atomic_ullong head;
	char head_pad[ATOMIC_RING_BUFFER_ALIGN - sizeof(atomic_ullong)];
	char data[0];
};

bool atomic_ring_buffer_is_empty(struct atomic_ring_buffer *queue);
void *atomic_ring_buffer_front(struct atomic_ring_buffer *queue);
void atomic_ring_buffer_pop(struct atomic_ring_buffer *queue, size_t element_size);
bool atomic_ring_buffer_emplace(struct atomic_ring_buffer *queue, void *element, size_t element_size);

#endif
