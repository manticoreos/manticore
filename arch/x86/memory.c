#include <kernel/memory.h>

#include <kernel/printf.h>
#include <kernel/align.h>

#include <stddef.h>
#include <stdint.h>

enum {
	COMMAND_LINE_TAG	= 1,
	BOOT_LOADER_NAME_TAG	= 2,
	MODULES_TAG		= 3,
	MEMORY_INFO_TAG		= 4,
	BIOS_BOOT_DEVICE_TAG	= 5,
	MEMORY_MAP_TAG		= 6,
	ELF_SYMBOLS_TAG		= 9,
};

struct header {
	uint32_t total_size;
	uint32_t reserved;
};

struct tag {
	uint32_t tag;
	uint32_t size;
};

struct memory_map {
	uint32_t entry_size;
	uint32_t entry_version;
};

struct memory_map_entry {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
};

static void parse_boot_loader_name(struct tag *tag, void *data)
{
	const char *boot_loader_name = data + sizeof(*tag);

	printf("Boot loader: '%s'\n", boot_loader_name);
}

static void parse_memory_map(struct tag *tag, void *data)
{
	size_t offset = sizeof(*tag);

	struct memory_map *memory_map = data + offset;
	offset += sizeof(*memory_map);

	printf("Memory map:\n");

	while (offset < tag->size) {
		struct memory_map_entry *entry = data + offset;
		if (entry->length < 1024*1024) {
			printf("  %016lx [%d KiB]\n", entry->base_addr, entry->length / 1024);
		} else {
			printf("  %016lx [%d MiB]\n", entry->base_addr, entry->length / 1024 / 1024);
		}
		offset += sizeof(*entry);
	}
}

/*
 * Boot data section provided by the Multiboot 2 compatible boot loader:
 */
extern void *boot_data;

void init_memory_map(void)
{
	void *data = boot_data;
	size_t offset = 0;

	struct header *header = data + offset;
	offset += sizeof(*header);

	while (offset < header->total_size) {
		struct tag *tag = data + offset;
		switch (tag->tag) {
		case BOOT_LOADER_NAME_TAG:
			parse_boot_loader_name(tag, data + offset);
			break;
		case MEMORY_MAP_TAG:
			parse_memory_map(tag, data + offset);
			break;
		default:
			break;
		}
		offset += tag->size;
		offset = align_up(offset, 8);
	}
}
