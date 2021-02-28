#include <kernel/platform.h>

#include <kernel/page-alloc.h>
#include <kernel/memory.h>
#include <kernel/printf.h>
#include <kernel/align.h>
#include <kernel/panic.h>

#include <stdbool.h>
#include "libfdt.h"

void *dtb;

void parse_platform_config(void)
{
	if (!dtb) {
		panic("No FDT found");
	}
	struct fdt_header *fdt = dtb;
	if (fdt_check_header(fdt) != 0) {
		panic("Bad FDT found");
	}
	printf("Found FDT at address %x\n", (unsigned long)dtb);
	int memory_node = fdt_path_offset(dtb, "/memory");
	if (memory_node < 0) {
		panic("No memory node found in FDT");
	}
	int addr_cells = fdt_address_cells(fdt, memory_node);
	if (addr_cells < 0) {
		panic("Bad address cells property");
	}
	int size_cells = fdt_size_cells(fdt, memory_node);
	if (size_cells < 0) {
		panic("Bad size cells property");
	}
	int size;
	const uint32_t *reg_prop = fdt_getprop(fdt, memory_node, "reg", &size);
	if (!reg_prop) {
		panic("No reg property found");
	}
	uint64_t memory_addr = 0;
	for (uint32_t i = 0; i < addr_cells; i++) {
		memory_addr = (memory_addr << 32) | fdt32_to_cpu(*reg_prop++);
	}
	uint64_t memory_size = 0;
	for (uint32_t i = 0; i < size_cells; i++) {
		memory_size = (memory_size << 32) | fdt32_to_cpu(*reg_prop++);
	}
	printf("Memory map:\n");
	printf("  %016lx-%016lx %4d MiB [available]\n", memory_addr, memory_addr + memory_size, memory_size / 1024 / 1024);

	extern char _kernel_end;
	uint64_t kernel_end = (uint64_t) &_kernel_end;

	if (memory_addr < kernel_end) {
		memory_size -= kernel_end - memory_addr;
		memory_addr = kernel_end;
	}
	memory_addr = align_up(memory_addr, PAGE_SIZE_SMALL);
	memory_size = align_down(memory_size, PAGE_SIZE_SMALL);
	memory_add_span(memory_addr, memory_size);
}
