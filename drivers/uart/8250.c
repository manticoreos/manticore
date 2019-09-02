/*
 * 8250/16550A UART device driver
 */

#include <kernel/console.h>

#include <arch/ioport.h>

/// \file drivers:uart:8250.c
/// An enum.
/// Enables UART dirvers read and write constants as described.


enum {
	UART_THR = 0, /* Write, DLAB=0 */
	UART_RBR = 0, /* Read, DLAB=0 */
	UART_DLL = 0, /* Read/Write, DLAB=1 */
	UART_IER = 1, /* Read/Write, DLAB=0 */
	UART_DLH = 1, /* Read/Write, DLAB=1 */
	UART_IIR = 2, /* Read */
	UART_FCR = 2, /* Write */
	UART_LCR = 3, /* Read/Write */
	UART_MCR = 4, /* Read/Write */
	UART_LSR = 5, /* Read */
	UART_MSR = 6, /* Read */
	UART_SR  = 7, /* Read/Write */
};

static uint16_t uart_8250_ioport = 0x3f8;

void uart_8250_write_char(char c)
{
	while (!(pio_read8(uart_8250_ioport + UART_LSR) & 0x20))
		;;

	if (c == '\r')
		c = '\n';
	pio_write8(c, uart_8250_ioport);
}

void console_write_char(char ch)
{
	uart_8250_write_char(ch);
}

void console_write_str(const char *s)
{
	while (*s) {
		uart_8250_write_char(*s++);
	}
}

void console_write(const char *s, size_t count)
{
	while (count--) {
		uart_8250_write_char(*s++);
	}
}

void console_init(void)
{
	/*
	 * Disable device interrupts:
	 */
	pio_write8(0x00, uart_8250_ioport + UART_IER);

	/*
	 * Set baud rate to 38400:
	 */
	pio_write8(0x80, uart_8250_ioport + UART_LCR);
	pio_write8(0x03, uart_8250_ioport + UART_DLL);
	pio_write8(0x00, uart_8250_ioport + UART_DLH);

	/*
	 * 8 bits, no parity, one stop bit:
	 */
	pio_write8(0x03, uart_8250_ioport + UART_LCR);

	/*
	 * Enable FIFO, clear them, interrupt trigger at 14-byte threshold:
	 */
	pio_write8(0xC7, uart_8250_ioport + UART_FCR);

	/*
	 * Enable IRQs, set RTS and DSR:
	 */
	pio_write8(0x0B, uart_8250_ioport + UART_MCR);
}
