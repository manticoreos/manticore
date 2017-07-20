#include <arch/ioport.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1

#define ICW1_IC4 (1U << 0)
#define ICW4_8086_MODE (1U << 0)

// Remap 8259 PIC interrupt vectors not to overlap with CPU exceptions.
void i8259_remap(void)
{
	/*
	 * Save interrupt masks:
	 */
	uint8_t pic1_mask = inb(PIC1_DATA);
	uint8_t pic2_mask = inb(PIC2_DATA);

	/*
	 * ICW1 - start initialization:
	 */
	outb(0x10 | ICW1_IC4, PIC1_CMD);
	outb(0x10 | ICW1_IC4, PIC2_CMD);

	/*
	 * ICW2 - interrupt vector offsets:
	 */
	outb(0x20, PIC1_DATA);
	outb(0x28, PIC2_DATA);

	/*
	 * ICW3 - cascading:
	 */
	outb(4, PIC1_DATA);
	outb(2, PIC2_DATA);

	/*
	 * ICW4 - other configuration:
	 */
	outb(ICW4_8086_MODE, PIC1_DATA);
	outb(ICW4_8086_MODE, PIC2_DATA);

	/*
	 * Restore interrupt masks:
	 */
	outb(pic1_mask, PIC1_DATA);
	outb(pic2_mask, PIC2_DATA);
}
