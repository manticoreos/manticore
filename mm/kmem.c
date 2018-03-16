//
// Kernel dynamic memory allocator
//
// The design of this kernel dynamic memory allocator mostly follows
// Bonwick's original paper on slab allocator:
//
// Bonwick, Jeff. "The Slab Allocator: An Object-Caching Kernel Memory
// Allocator." USENIX summer. Vol. 16. 1994.
//

#include <kernel/kmem.h>

#include <kernel/align.h>
#include <kernel/errno.h>
#include <kernel/page-alloc.h>
#include <kernel/printf.h>

#include <stdbool.h>
#include <string.h>

static struct kmem_cache kmem_cache_cache;
static struct kmem_cache kmem_slab_cache;

static struct kmem_bufctl *kmem_object_to_bufctl(struct kmem_cache *cache, void *obj)
{
	return obj + cache->bufctl;
}

static struct kmem_slab *kmem_slab_create(struct kmem_cache *cache)
{
	void *base = page_alloc_small();
	if (!base) {
		return NULL;
	}
	size_t slab_size = PAGE_SIZE_SMALL;
	struct kmem_slab *slab;
	if (cache->size < PAGE_SIZE_SMALL / 8) {
		slab = base + PAGE_SIZE_SMALL - sizeof(*slab);
		slab_size -= sizeof(*slab);
	} else {
		slab = kmem_cache_alloc(&kmem_slab_cache);
		if (!slab) {
			goto error_free_page;
		}
	}
	slab->cache = cache;
	slab->base = base;
	slab->head = kmem_object_to_bufctl(cache, base);
	slab->next = NULL;
	slab->nr_free = 0;
	slab->capacity = 0;
	size_t buffer_size = align_up(cache->size, cache->align);
	for (size_t offset = 0; offset + buffer_size <= slab_size; offset += buffer_size) {
		void *addr = base + offset;
		struct kmem_bufctl *bufctl = kmem_object_to_bufctl(cache, addr);
		size_t next_offset = offset + buffer_size;
		if (next_offset + buffer_size <= slab_size) {
			bufctl->next = kmem_object_to_bufctl(cache, base + next_offset);
		} else {
			bufctl->next = NULL;
		}
		bufctl->addr = addr;
		bufctl->slab = slab;
		slab->capacity++;
		slab->nr_free++;
	}
	return slab;
error_free_page:
	page_free_small(base);
	return NULL;
}

static void kmem_slab_destroy(struct kmem_slab *slab)
{
	page_free_small(slab->base);
}

static void *kmem_slab_alloc_object(struct kmem_slab *slab)
{
	struct kmem_bufctl *bufctl = slab->head;
	if (!bufctl) {
		return NULL;
	}
	slab->head = bufctl->next;
	slab->nr_free--;
	return bufctl->addr;
}

static bool kmem_slab_is_full(struct kmem_slab *slab)
{
	return slab->nr_free == slab->capacity;
}

static void kmem_slab_free_object(struct kmem_slab *slab, void *obj)
{
	struct kmem_bufctl *bufctl = kmem_object_to_bufctl(slab->cache, obj);
	bufctl->next = slab->head;
	bufctl->addr = obj;
	bufctl->slab = slab;
	slab->head = bufctl;
	slab->nr_free++;
}

static int kmem_cache_init(struct kmem_cache *cache, const char *name, size_t size, size_t align)
{
	if (align_up(size, align) < sizeof(struct kmem_bufctl)) {
		return -EINVAL;
	}
	strlcpy(cache->name, name, KMEM_NAME_MAX_LEN);
	cache->size = size;
	cache->align = align;
	cache->bufctl = align_up(size, align) - sizeof(struct kmem_bufctl);
	cache->slab = kmem_slab_create(cache);
	return 0;
}

struct kmem_cache *kmem_cache_create(const char *name, size_t size, size_t align)
{
	struct kmem_cache *cache = kmem_cache_alloc(&kmem_cache_cache);
	if (cache) {
		if (kmem_cache_init(cache, name, size, align) < 0) {
			kmem_cache_free(&kmem_cache_cache, cache);
			return NULL;
		}
	}
	return cache;
}

void kmem_cache_destroy(struct kmem_cache *cache)
{
	struct kmem_slab *slab = cache->slab;
	for (;;) {
		struct kmem_slab *next = slab->next;
		kmem_slab_destroy(slab);
		if (!next) {
			break;
		}
		slab = next;
	}
	kmem_cache_free(&kmem_cache_cache, cache);
}

void *kmem_cache_alloc(struct kmem_cache *cache)
{
	for (;;) {
		void *obj = kmem_slab_alloc_object(cache->slab);
		if (obj) {
			return obj;
		}
		struct kmem_slab *slab = kmem_slab_create(cache);
		if (!slab) {
			return NULL;
		}
		slab->next = cache->slab;
		cache->slab = slab;
	}
}

void kmem_cache_free(struct kmem_cache *cache, void *obj)
{
	kmem_slab_free_object(cache->slab, obj);

	if (!kmem_slab_is_full(cache->slab)) {
		return;
	}
	struct kmem_slab *slab = cache->slab;
	if (slab->next) {
		cache->slab = slab->next;
		kmem_slab_destroy(slab);
	}
}

static size_t kmem_alloc_sizes[] = {
    32, 64, 128, 256, 512, 1024, 2048, 4096,
};

static struct kmem_cache kmalloc_caches[ARRAY_SIZE(kmem_alloc_sizes)];

static inline size_t kmem_cache_index(size_t size)
{
	if (size <= 32)
		return 0;
	if (size <= 64)
		return 1;
	if (size <= 128)
		return 2;
	if (size <= 256)
		return 3;
	if (size <= 512)
		return 4;
	if (size <= 1024)
		return 5;
	if (size <= 2048)
		return 6;
	if (size <= 4096)
		return 7;
	return ~0ULL;
}

static inline struct kmem_cache *kmem_size_to_cache(size_t size)
{
	size_t idx = kmem_cache_index(size);
	if (idx >= ARRAY_SIZE(kmem_alloc_sizes)) {
		return NULL;
	}
	return &kmalloc_caches[idx];
}

void *kmem_alloc(size_t size)
{
	struct kmem_cache *cache = kmem_size_to_cache(size);
	if (!cache) {
		return NULL;
	}
	return kmem_cache_alloc(cache);
}

void *kmem_zalloc(size_t size)
{
	void *p = kmem_alloc(size);
	if (!p) {
		return NULL;
	}
	memset(p, 0, size);
	return p;
}

void kmem_free(void *ptr, size_t size)
{
	struct kmem_cache *cache = kmem_size_to_cache(size);
	if (!cache) {
		return;
	}
	kmem_cache_free(cache, ptr);
}

int kmem_init(void)
{
	int err;
	err = kmem_cache_init(&kmem_cache_cache, "kmem_cache_cache", sizeof(struct kmem_cache), KMEM_DEFAULT_ALIGN);
	if (err) {
		return err;
	}
	err = kmem_cache_init(&kmem_slab_cache, "kmem_slab_cache", sizeof(struct kmem_slab), KMEM_DEFAULT_ALIGN);
	if (err) {
		return err;
	}
	for (unsigned int i = 0; i < ARRAY_SIZE(kmem_alloc_sizes); i++) {
		char cache_name[KMEM_NAME_MAX_LEN];
		size_t size = kmem_alloc_sizes[i];
		snprintf(cache_name, KMEM_NAME_MAX_LEN, "kmalloc-%lu", size);
		err = kmem_cache_init(&kmalloc_caches[i], cache_name, size, KMEM_DEFAULT_ALIGN);
		if (err) {
			return err;
		}
	}
	return 0;
}
