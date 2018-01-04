#ifndef KERNEL_KMEM_H
#define KERNEL_KMEM_H

#include <stddef.h>
#include <stdint.h>

/// Default object alignment.
#define KMEM_DEFAULT_ALIGN sizeof(void *)

/// A buffer metadata block.
struct kmem_bufctl {
	struct kmem_bufctl	*next;
	void			*addr;
	struct kmem_slab	*slab;
};

/// An object cache slab.
struct kmem_slab {
	struct kmem_cache	*cache;
	struct kmem_bufctl	*head;
	void			*base;
	struct kmem_slab	*next;
	uint32_t		nr_free;
	uint32_t		capacity;
};

/// Maximum cache name length.
#define KMEM_NAME_MAX_LEN	32

/// An object cache.
struct kmem_cache {
	size_t			bufctl;		// Bufctl offset
	size_t			size;		// Object size
	size_t			align;		// Object alignment
	struct kmem_slab	*slab;		// Slab
	char			name[KMEM_NAME_MAX_LEN]; // Cache name
};

/// Create an object cache.
struct kmem_cache *kmem_cache_create(const char *name, size_t obj_size, size_t align);

/// Destroy an object cache.
void kmem_cache_destroy(struct kmem_cache *cache);

/// Allocate an object from \cache object cache.
void *kmem_cache_alloc(struct kmem_cache *cache);

/// Free an object to \cache object cache.
void kmem_cache_free(struct kmem_cache *cache, void *obj);

/// Allocate an object of size \size.
void *kmem_alloc(size_t size);

/// Allocate an object of size \size and fill the memory with zeros.
void *kmem_zalloc(size_t size);

/// Free an object \ptr of size \size.
void kmem_free(void *ptr, size_t size);

/// Initialize the kernel memory allocator.
void kmem_init(void);

#endif
