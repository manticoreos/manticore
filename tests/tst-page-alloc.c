#include <kernel/page-alloc.h>
#include <kernel/printf.h>
#include <string.h>

static void test_page_alloc_small(void)
{
	printf("%s\n", __func__);
	void *page1 = page_alloc_small();
	void *page2 = page_alloc_small();
	void *page3 = page_alloc_small();
	page_free_small(page1);
	page_free_small(page2);
	page_free_small(page3);
	for (;;) {
		void *page = page_alloc_small();
		printf("page: %p\n", page);
		if (!page) {
			printf("out of memory\n");
			break;
		}
		memset(page, 0xfe, PAGE_SIZE_SMALL);
	}
}

static void test_page_alloc_large(void)
{
	printf("%s\n", __func__);
	void *page1 = page_alloc_large();
	void *page2 = page_alloc_large();
	void *page3 = page_alloc_large();
	page_free_large(page1);
	page_free_large(page2);
	page_free_large(page3);
	for (;;) {
		void *page = page_alloc_large();
		printf("page: %p\n", page);
		if (!page) {
			printf("out of memory\n");
			break;
		}
		memset(page, 0xfe, PAGE_SIZE_LARGE);
	}
}

void test_page_alloc(void)
{
	test_page_alloc_small();
	test_page_alloc_large();
}
