#include <kernel/memory.h>

#include <kernel/printf.h>
#include <kernel/panic.h>

#include <stdbool.h>
#include "libfdt.h"

void *dtb;

void init_memory_map(void)
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
	uint32_t *reg_prop = fdt_getprop(fdt, memory_node, "reg", &size);
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
	printf("  %016lx %d MiB\n", memory_addr, memory_size / 1024 / 1024);

}
