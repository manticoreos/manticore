#ifndef KERNEL_ATOMIC_RING_BUFFER_QUEUE_H
#define KERNEL_ATOMIC_RING_BUFFER_QUEUE_H

#include <manticore/atomic_ring_buffer_abi.h>

#include <stdbool.h>

bool atomic_ring_buffer_is_empty(struct atomic_ring_buffer *queue);
void *atomic_ring_buffer_front(struct atomic_ring_buffer *queue);
void atomic_ring_buffer_pop(struct atomic_ring_buffer *queue);
bool atomic_ring_buffer_emplace(struct atomic_ring_buffer *queue, void *element);

#endif
