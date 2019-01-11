#ifndef X86_APIC_H
#define X86_APIC_H

#include <stdint.h>

struct msi_message {
	uint64_t msg_addr;
	uint32_t msg_data;
};

void apic_compose_msi_msg(struct msi_message *msg, uint8_t vector, uint8_t dest_id);

void init_apic(void);

#endif
