#ifndef KERNEL_PAGE_ALLOC_H
#define KERNEL_PAGE_ALLOC_H

#include <stdint.h>

#define PAGE_SIZE_SMALL (1ULL << 12)
#define PAGE_SIZE_LARGE (1ULL << 21)


/// \file       include:kernel:page-alloc.h
/// \brief      File constitutes page memory allocation and deallocation methods
///
/// \fn         page_alloc_init(void)
/// \brief      page_alloc_init initialzes the page allocation
///
/// \fn         page_alloc_small(void)
/// \brief      page_alloc_small allocates a small page
///
/// \fn         page_free_small(void *page)
/// \brief      page_free_small free allocated page memory
/// \param page page holds memory address
///
/// \fn         *page_alloc_large(void)
/// \brief      page_alloc_large allocates lage page
///
/// \fn         page_free_large(void *page)
/// \brief      page_free_large deallocates large page allocated
/// \param page page holds memory address

void page_alloc_init(void);
void *page_alloc_small(void);
void page_free_small(void *page);
void *page_alloc_large(void);
void page_free_large(void *page);

#endif
