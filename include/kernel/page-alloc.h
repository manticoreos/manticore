#ifndef KERNEL_PAGE_ALLOC_H
#define KERNEL_PAGE_ALLOC_H

#include <stdint.h>

#define PAGE_SIZE_SMALL (1ULL << 12)
#define PAGE_SIZE_LARGE (1ULL << 21)

void page_alloc_init(void);
void *page_alloc_small(void);
void page_free_small(void *page);
void *page_alloc_large(void);
void page_free_large(void *page);

#endif
