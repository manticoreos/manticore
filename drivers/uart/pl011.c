/*
 * PL011 UART driver
 *
 * References:
 *
 *   PrimeCell UART (PL011) Technical Reference Manual (Revision: r1p5)
 */

#include <kernel/console.h>

/* UART registers. See Table 3-1 ("UART register summary") of the specification */
enum {
	UARTDR		= 0x000,
};

/* UART registers base address */
static volatile char *pl011_regs = (char *) 0x9000000;

void pl011_write_char(int ch)
{
	pl011_regs[UARTDR] = ch;
}

void console_write_char(char ch)
{
	pl011_write_char(ch);
}

void console_write(const char *s)
{
	while (*s) {
		pl011_write_char(*s++);
	}
}

void console_init(void)
{
}
