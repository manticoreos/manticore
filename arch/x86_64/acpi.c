#include <arch/vmem.h>

#include <kernel/printf.h>

#include <assert.h>
#include <stdint.h>
#include <string.h>

#define ACPI_RSDP_SIG "RSD PTR "
#define ACPI_RSDP_SIG_LEN 8

/* ACPI Root System Description Pointer (RSDP) descriptor.  */
struct acpi_rsdp_desc {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
};

/* ACPI System Description Table header.  */
struct acpi_sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

/* Multiple APIC Description Table (MADT).  */
struct acpi_madt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
	uint32_t local_ic_addr;
	uint32_t flags;
};

enum {
	ACPI_PROCESSOR_LOCAL_APIC = 0x00,
	ACPI_IO_APIC = 0x01,
	ACPI_INT_SOURCE_OVERRIDE = 0x02,
	ACPI_NMI_SOURCE = 0x03,
	ACPI_LOCAL_APIC_NMI = 0x04,
};

static uint8_t acpi_read_u8(void *ptr)
{
	return *((uint8_t *) ptr);
}

static uint32_t acpi_read_u32(void *ptr)
{
	return *((uint32_t *) ptr);
}

static void acpi_parse_madt(void *raw_madt)
{
	struct acpi_madt_header *madt_header = raw_madt;

	if (memcmp(madt_header->signature, "APIC", 4)) {
		printf("ACPI MADT signature mismatch\n");
		return;
	}

	size_t nr_cpus = 0;

	size_t madt_len = madt_header->length;
	size_t off = sizeof(struct acpi_madt_header);
	while (off < madt_len) {
		uint8_t type = acpi_read_u8(raw_madt + off);
		uint8_t len = acpi_read_u8(raw_madt + off + 1);

		switch (type) {
		case ACPI_PROCESSOR_LOCAL_APIC:
			nr_cpus++;
			break;
		case ACPI_IO_APIC:
			break;
		case ACPI_INT_SOURCE_OVERRIDE:
			break;
		case ACPI_LOCAL_APIC_NMI:
			break;
		default:
			printf("warning: unknown ACPI MADT entry: %d\n", type);
			return;
		}
		off += len;
	}
	printf("Found %lu CPUs via ACPI MADT\n", nr_cpus);
}

/* Parse platform configuration from the given ACPI Root System Description
   Pointer (RSDP).  */
void acpi_parse_config(void *raw_rsdp)
{
	struct acpi_rsdp_desc *rsdp = raw_rsdp;

	if (memcmp(rsdp->signature, ACPI_RSDP_SIG, ACPI_RSDP_SIG_LEN)) {
		printf("ACPI RSDP signature mismatch\n");
		return;
	}

	void *rsdt_addr = (void *) (uint64_t) rsdp->rsdt_address;

	struct acpi_sdt_header *rsdt_header = rsdt_addr;

	if (memcmp(rsdt_header->signature, "RSDT", 4)) {
		printf("ACPI RSDT signature mismatch\n");
		return;
	}

	for (size_t off = sizeof(struct acpi_sdt_header); off < rsdt_header->length; off += 4) {
		uint32_t sdt_addr = acpi_read_u32(rsdt_addr + off);

		struct acpi_sdt_header *sdt_header = (void *) (uint64_t) sdt_addr;

		if (!memcmp(sdt_header->signature, "APIC", 4)) {
			acpi_parse_madt(sdt_header);
		}
	}
}
