#ifndef KERNEL_PAGE_ALLOC_H
#define KERNEL_PAGE_ALLOC_H

#include <stdint.h>

#define PAGE_SIZE_2M (1ULL << 21)

void page_alloc_init(void);

void *page_alloc(void);

void page_free(void *page);

#endif
