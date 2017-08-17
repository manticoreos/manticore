#include <kernel/kmem.h>

#include <string.h>

void test_kmem(void)
{
	struct kmem_cache *cache = kmem_cache_create("cache", 64, KMEM_DEFAULT_ALIGN);
	for (int i = 0; i < 10000; i++) {
		void *p = kmem_cache_alloc(cache);
		if (!p) {
			break;
		}
		memset(p, 0xfe, 64);
		kmem_cache_free(cache, p);
		p = kmem_cache_alloc(cache);
		if (!p) {
			break;
		}
		memset(p, 0xfe, 64);
	}
	kmem_cache_destroy(cache);
}
