#ifndef __MANTICORE_UAPI_ATOMIC_RING_BUFFER_ABI_H
#define __MANTICORE_UAPI_ATOMIC_RING_BUFFER_ABI_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

// Alignment for atomic ring buffer metadata elements (e.g. head and tail
// pointers). Basically, aligning to L1 cache line size to avoid false sharing.
#define ATOMIC_RING_BUFFER_ALIGN 64

// An atomic ring buffer.
struct atomic_ring_buffer {
	size_t element_size;
	size_t capacity;
	char header_padding[ATOMIC_RING_BUFFER_ALIGN - sizeof(size_t) - sizeof(size_t)];
	atomic_ullong tail;
	char tail_pad[ATOMIC_RING_BUFFER_ALIGN - sizeof(atomic_ullong)];
	atomic_ullong head;
	char head_pad[ATOMIC_RING_BUFFER_ALIGN - sizeof(atomic_ullong)];
	char data[0];
};

#endif
