/*
 * malloc() is a general-purpose dynamic memory allocator.
 *
 * The design of this allocator is inspired by mimalloc.
 *
 * Layout in memory looks as follows.
 *
 *                               Heap
 *               Segment 1                   Segment 2
 *     +--------------------------+ +--------------------------+
 *     | Page 1 | Page 2 | Page 3 | | Page 4 | Page 5 | Page 6 |
 *     +--------------------------+ +--------------------------+
 *     | B B B  | B B B  | B B B  | | B B B  | B B B  | B B B  |
 *     +--------------------------+ +--------------------------+
 *
 * The allocator maintains one or more heaps, which consists of segments that
 * are divided into pages. Each segment is 2 MiB (a large page) of contiguous
 * virtual memory, which is divided into 64 KiB pages. Each page is divided
 * into equal-sized blocks, which hold memory for objects.
 *
 * The malloc() interface allocates from the default heap. Users can create as
 * many heaps as they want and allocate from them with the
 * malloc_heap_alloc() function.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __MANTICORE_OS__ 
#include <manticore/syscalls.h>
#include <manticore/vmspace_abi.h>
#else
#include <sys/mman.h>
#endif

/* Enable malloc benchmark.  */
//#define MALLOC_BENCHMARK 1

/* Enable malloc tracing.  */
//#define MALLOC_TRACING 1

/* Enable malloc test.  */
//#define MALLOC_TEST 1

#ifdef MALLOC_TRACING
#define MALLOC_TRACE(fmt, ...)                                                                                         \
	do {                                                                                                           \
		printf("%s:%d: %s(" fmt ")\n", __FILE__, __LINE__, __func__, __VA_ARGS__);                             \
	} while (0)
#else
#define MALLOC_TRACE(fmt, ...)                                                                                         \
	do {                                                                                                           \
	} while (0)
#endif

#define malloc_likely(x) __builtin_expect((x), 1)
#define malloc_unlikely(x) __builtin_expect((x), 0)

#define malloc_align(value, align) ((value + align - 1) & ~(align - 1))

/* The minimum object size.  */
#define MIN_OBJ_SIZE_SHIFT 3
#define MIN_OBJ_SIZE (1ULL << MIN_OBJ_SIZE_SHIFT)
#define MIN_OBJ_SIZE_MASK (MIN_OBJ_SIZE - 1)

/* The size of a malloc segment.  */
#define MALLOC_SEGMENT_SIZE (1UL << 21) /* 2 MiB */

/* The size of a malloc page.  */
#define MALLOC_PAGE_SIZE (1UL << 16) /* 64 KiB */

/* The maximum size of a small object.  */
#define SMALL_OBJ_SIZE_MAX (1UL << 10) /* 1 KiB */

/* Number of size classes in heap.  */
#define NUM_SIZE_CLASSES ((SMALL_OBJ_SIZE_MAX + 7) >> MIN_OBJ_SIZE_SHIFT)

/* A block of memory that is allocated with malloc() and de-allocated
   with free(). Each block belongs to a single page.  */
struct malloc_block {
	struct malloc_block *next;
};

/* A page is a region in memory that holds one or more equal-sized
   blocks of memory. Each page belongs to a single segment.  */
struct malloc_page {
	struct malloc_block *free;
	size_t used;
	size_t capacity;
	size_t block_size;
	uint8_t idx;
	/* FIXME: hole in struct! */
};

/* A segment is a region in memory that holds one or more pages.  */
struct malloc_segment {
	struct malloc_segment *prev;
	struct malloc_segment *next;
	size_t nr_pages;
	struct malloc_page pages[]; // pages within this segment
};

/* A segment queue holds one more more segments that have free pages.  */
struct malloc_segment_queue {
	struct malloc_segment *head;
	struct malloc_segment *tail;
};

/* A heap is a region in memory that holds one more more segments.  */
struct malloc_heap {
	struct malloc_page *pages[NUM_SIZE_CLASSES];
	struct malloc_segment_queue segment_queue;
};

static struct malloc_heap malloc_default_heap;

/* The heap has pages for each size-class for quick allocation. The pages are
   initially set to this empty page so that we allocate actual pages lazily.  */
static struct malloc_page empty_page = {
	.block_size	= 0,
	.capacity	= 0,
	.used		= 0,
	.free		= NULL,
};

/* Convert an object size to a block size.  */
static inline size_t to_block_size(size_t obj_size)
{
	return (obj_size + MIN_OBJ_SIZE_MASK) & ~(MIN_OBJ_SIZE_MASK); // FIXME: use malloc_align()
}

/* Convert an object size to a size class.  */
static inline size_t to_size_class(size_t obj_size)
{
	return to_block_size(obj_size) / MIN_OBJ_SIZE;
}

/* Convert a malloc block to an object.  */
static void *malloc_block_to_obj(struct malloc_block *block)
{
	return (void *) block;
}

/* Convert a malloc object to a block.  */
static struct malloc_block *malloc_obj_to_block(void *obj)
{
	return obj;
}

/* Number of remaining free objects in a page.  */
static size_t malloc_page_remaining(struct malloc_page *page)
{
	return page->capacity - page->used;
}

/* Look up the page of an object.  */
static struct malloc_page *malloc_obj_to_page(struct malloc_segment *segment, void *obj)
{
	size_t idx = ((void *) obj - (void *) segment) / MALLOC_PAGE_SIZE;

	return &segment->pages[idx];
}

/* Append object to the page freelist.  */
static void malloc_page_append_obj(struct malloc_page *page, void *obj)
{
	struct malloc_block *block = malloc_obj_to_block(obj);
	block->next = page->free;
	page->free = block;
}

/* Allocate an object from `page`.  */
static void *malloc_page_alloc_obj(struct malloc_page *page)
{
	struct malloc_block *block = page->free;
	if (block) {
		page->free = block->next;
		page->used++;
		return malloc_block_to_obj(block);
	}
	return NULL;
}

/* Check if `page` is full and has no more available blocks.  */
static bool malloc_page_full(struct malloc_page *page)
{
	return page->free == NULL;
}

/* Look up the segment of an object.  */
static struct malloc_segment *malloc_obj_to_segment(void *obj)
{
	return obj - ((unsigned long) obj % MALLOC_SEGMENT_SIZE);
}

/* Return the size of `segment` metadata.  */
static size_t malloc_segment_metadata_size(struct malloc_segment *segment)
{
	// TODO: Align the end of segment metadata?
	return sizeof(struct malloc_segment) + (sizeof(struct malloc_page) * segment->nr_pages);
}

/* Return the object buffer for `page` in `segment`.

   Please note that the first page in segment is smaller than rest of the pages
   because segment begins with a metadata section, which steals space from the
   first page.  */
static void *malloc_segment_page_obj(struct malloc_segment *segment, struct malloc_page *page)
{
	void *segment_start = segment;
	if (page->idx == 0) {
		return segment_start + malloc_segment_metadata_size(segment);
	}
	return segment_start + (page->idx * MALLOC_PAGE_SIZE);
}

/* Return the size of `page` in `segment`.

   Please note that the first page in segment is smaller than rest of the pages
   because segment begins with a metadata section, which steals space from the
   first page.  */
static size_t malloc_segment_page_size(struct malloc_segment *segment, struct malloc_page *page)
{
	if (page->idx == 0) {
		return MALLOC_PAGE_SIZE - malloc_segment_metadata_size(segment);
	}
	return MALLOC_PAGE_SIZE;
}

/* Initialize `segment` data structure.  */
static void malloc_segment_init(struct malloc_segment *segment, size_t segment_size)
{
	MALLOC_TRACE("segment=%p, segment_size=%ld", segment, segment_size);

	segment->prev = NULL;
	segment->next = NULL;
	segment->nr_pages = segment_size / MALLOC_PAGE_SIZE;

	for (size_t page_idx = 0; page_idx < segment->nr_pages; page_idx++) {
		struct malloc_page *page = &segment->pages[page_idx];

		page->idx = page_idx;
	}
}

static void *malloc_os_alloc(size_t size, size_t align)
{
	MALLOC_TRACE("size=%lu, align=%lu", size, align);

#ifdef __MANTICORE_OS__
	struct vmspace_region vmr = {
		.size = MALLOC_SEGMENT_SIZE,
		.align = MALLOC_SEGMENT_SIZE,
	};
	if (vmspace_alloc(&vmr, sizeof(vmr)) != 0) {
		return NULL;
	}
	assert((vmr.start % MALLOC_SEGMENT_SIZE) == 0);
	return (void *) vmr.start;
#else
	void *p = mmap(NULL, size + align, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (p == MAP_FAILED) {
		return NULL;
	}
	// TODO: Virtual memory up to "align" is wasted here, but it's expensive to munmap it so...
	void *q = (void*) malloc_align((size_t) p, align);
	return q;
#endif
}

/* Allocate a new segment from the OS.  */
static struct malloc_segment *malloc_heap_alloc_segment(struct malloc_heap *heap)
{
	MALLOC_TRACE("heap=%p", heap);

	void *p = malloc_os_alloc(MALLOC_SEGMENT_SIZE, MALLOC_SEGMENT_SIZE);
	if (!p) {
		return NULL;
	}
	struct malloc_segment *segment = p;
	malloc_segment_init(segment, MALLOC_SEGMENT_SIZE);
	return segment;
}

/* Return `segment queue` head.  */
static struct malloc_segment *malloc_segment_queue_head(struct malloc_segment_queue *queue)
{
	return queue->head;
}

static void malloc_segment_queue_remove(struct malloc_segment_queue *queue, struct malloc_segment *segment)
{
	if (segment == queue->head) {
		queue->head = segment->next;
	}
	if (segment == queue->tail) {
		queue->tail = segment->prev;
	}
	if (segment->prev) {
		segment->prev->next = segment->next;
		segment->prev = NULL;
	}
	if (segment->next) {
		segment->next->prev = segment->prev;
		segment->next = NULL;
	}
}

/* Remove `segment queue` head.  */
static void malloc_segment_queue_pop(struct malloc_segment_queue *queue)
{
	if (queue->head)
		malloc_segment_queue_remove(queue, queue->head);
}

/* Enqueue `segment` to `queue`.  */
static void malloc_segment_queue_enqueue(struct malloc_segment_queue *queue, struct malloc_segment *segment)
{
	segment->prev = queue->tail;
	segment->next = NULL;
	if (queue->tail) {
		queue->tail->next = segment;
		queue->tail = segment;
	} else {
		queue->head = queue->tail = segment;
	}
}

/* Claim a `page` from `segment` for given `block_size`.

   When this function completes, `page` can only be used to satisfy allocations
   for `block_size` bytes.  */
static void malloc_segment_claim_page(struct malloc_segment *segment, struct malloc_page *page, size_t block_size)
{
	MALLOC_TRACE("segment=%p, page=%p, block_size=%lu", segment, page, block_size);

	assert(page->block_size == 0); // unclaimed page

	page->block_size = block_size;
	page->used = 0;
	page->capacity = malloc_segment_page_size(segment, page) / block_size;

	/* The page freelist is a singly linked list. Add blocks in reverse
	   order to ensure that the block with the smallest address (beginning
	   of page) is in the front of the list.  */
	void *obj = malloc_segment_page_obj(segment, page);
	for (size_t idx = 0; idx < page->capacity; idx++) {
		malloc_page_append_obj(page, obj + ((page->capacity - idx - 1) * block_size));
	}
}

/* Allocate a new page from `heap` that can satisfy an allocation for `size`
   bytes.  */
static struct malloc_page *malloc_heap_alloc_page(struct malloc_heap *heap, size_t size)
{
	MALLOC_TRACE("heap=%p, size=%lu", heap, size);

	struct malloc_segment *segment;
retry:
	segment = malloc_segment_queue_head(&heap->segment_queue);
	if (!segment) {
		segment = malloc_heap_alloc_segment(heap);
		if (!segment) {
			return NULL; /* out of memory */
		}
		malloc_segment_queue_enqueue(&heap->segment_queue, segment);
	}
	size_t block_size = to_block_size(size);
	/* FIXME: replace segment->nr_pages traversal with page queue to speed it up.  */
	for (int i = 0; i < segment->nr_pages; i++) {
		struct malloc_page *page = &segment->pages[i];

		if (!page->block_size) {
			malloc_segment_claim_page(segment, page, block_size);
		} else if (page->block_size != block_size) {
			continue;
		}
		if (malloc_page_remaining(page)) {
			// TODO: remove segment from free queue when there are no pages with free blocks.
			return page;
		}
	}
	malloc_segment_queue_pop(&heap->segment_queue);
	goto retry;
}

/* Find or allocate a page from `heap` which can satisfy allocation for `size` bytes.  */
static struct malloc_page *malloc_heap_get_page(struct malloc_heap *heap, size_t size)
{
	MALLOC_TRACE("heap=%p, size=%lu", heap, size);

	size_t size_class = to_size_class(size);
	struct malloc_page *page = heap->pages[size_class];
	if (!malloc_page_full(page)) {
		return page;
	}
	page = malloc_heap_alloc_page(heap, size);
	if (page) {
		heap->pages[size_class] = page;
	}
	return page;
}

/* Allocate `size` bytes of memory from the small object pool of `heap`.  */
static void *malloc_heap_alloc_small(struct malloc_heap *heap, size_t size)
{
	MALLOC_TRACE("heap=%p, size=%lu", heap, size);

	if (!size) {
		size = sizeof(void *);
	}
	struct malloc_page *page = malloc_heap_get_page(heap, size);
	if (!page) {
		return NULL;
	}
	return malloc_page_alloc_obj(page);
}

/* Allocate `size` bytes of memory from `heap`.  */
static void *malloc_heap_alloc(struct malloc_heap *heap, size_t size)
{
	MALLOC_TRACE("heap=%p, size=%lu", heap, size);

	if (malloc_likely(size < SMALL_OBJ_SIZE_MAX)) {
		return malloc_heap_alloc_small(heap, size);
	}
	/* TODO: support for medium and large size allocations */
	return NULL;
}

/* Allocate `size` bytes of memory. */
void *malloc(size_t size)
{
	MALLOC_TRACE("size=%lu", size);

	return malloc_heap_alloc(&malloc_default_heap, size);
}

/* Free memory pointed to by `ptr` in `heap`.  */
static void malloc_heap_free(struct malloc_heap *heap, void *ptr)
{
	MALLOC_TRACE("heap=%p, ptr=%p", heap, ptr);

	struct malloc_segment *segment = malloc_obj_to_segment(ptr);
	struct malloc_page *page = malloc_obj_to_page(segment, ptr);
	// FIXME: append to page local freelist like in mimalloc
	malloc_page_append_obj(page, ptr);
	assert(page->used > 0);
	page->used--;
	if (!page->used) {
		/* TODO: free page */
	}
}

/* Free memory pointed to by `ptr`.  */
void free(void *ptr)
{
	MALLOC_TRACE("ptr=%p", ptr);

	if (!ptr) {
		return;
	}
	malloc_heap_free(&malloc_default_heap, ptr);
}

/* Initialize `heap` data structure.  */
static void malloc_heap_init(struct malloc_heap *heap)
{
	MALLOC_TRACE("heap=%p", heap);

	for (size_t idx = 0; idx < NUM_SIZE_CLASSES; idx++) {
		heap->pages[idx] = &empty_page;
	}
}

static void malloc_test(void)
{
#ifdef MALLOC_TEST
	/* malloc() recycles the same object on malloc/free */
	void *q = malloc(5);
	free(q);
	for (int i = 0; i < 100; i++) {
		void *p = malloc(5);
		assert(p == q);
		free(p);
	}

	/* malloc across different sizes */
	for (int i = 0; i < 100; i++) {
		void *p = malloc(i * 5);
		assert(p != NULL);
		free(p);
	}

	/* allocate lots of objects */
	for (int i = 0; i < 10000000; i++) {
		void *p = malloc(64);
		assert(p != NULL);
		printf("%p\n", p);
	}
	assert(0);
#endif
}

/* Initialize the dynamic memory allocator.

   Programs are expected to call this function before calling `malloc`, `free`,
   and other dynamic memory allocator APIs.  */
void __liblinux_malloc_init(void)
{
	malloc_heap_init(&malloc_default_heap);

	malloc_test();
}

#ifdef MALLOC_BENCHMARK
#include <time.h>

static uint64_t timespec_to_ns(struct timespec *ts)
{
	return ts->tv_sec * 1e9 + ts->tv_nsec;
}

static uint64_t time_diff(struct timespec *start, struct timespec *end)
{
	uint64_t start_ns = timespec_to_ns(start);
	uint64_t end_ns = timespec_to_ns(end);
	assert(start_ns < end_ns);
	return end_ns - start_ns;
}


int main(int argc, char *argv[])
{
	__liblinux_malloc_init();

	struct timespec start, end;

	if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
		assert(0);
	}
	int nr_iter = 10000000;
	for (int i = 0; i < nr_iter; i++) {
		void *p = malloc(64);
		assert(p != NULL);
	}
	if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
		assert(0);
	}
	printf("%.2f ns/call\n", (double) time_diff(&start, &end) / (double) nr_iter);
}
#endif
