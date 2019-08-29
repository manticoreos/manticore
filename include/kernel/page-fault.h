#ifndef KERNEL_PAGE_FAULT_H
#define KERNEL_PAGE_FAULTH

void page_fault_set_fixup(void *);
void *page_fault_get_fixup(void);

#endif
