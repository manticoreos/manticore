/*
 * 8250/16550A UART device driver
 */

#include <kernel/console.h>

#include <arch/ioport.h>

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
	while (!(inb(uart_8250_ioport + UART_LSR) & 0x20))
		;;

	if (c == '\r')
		c = '\n';
	outb(c, uart_8250_ioport);
}

void console_write_char(char ch)
{
	uart_8250_write_char(ch);
}

void console_write(const char *s)
{
	while (*s) {
		uart_8250_write_char(*s++);
	}
}

void console_init(void)
{
	/*
	 * Disable device interrupts:
	 */
	outb(uart_8250_ioport + UART_IER, 0x00);

	/*
	 * Set baud rate to 38400:
	 */
	outb(uart_8250_ioport + UART_LCR, 0x80);
	outb(uart_8250_ioport + UART_DLL, 0x03);
	outb(uart_8250_ioport + UART_DLH, 0x00);

	/*
	 * 8 bits, no parity, one stop bit:
	 */
	outb(uart_8250_ioport + UART_LCR, 0x03);

	/*
	 * Enable FIFO, clear them, interrupt trigger at 14-byte threshold:
	 */
	outb(uart_8250_ioport + UART_FCR, 0xC7);

	/*
	 * Enable IRQs, set RTS and DSR:
	 */
	outb(uart_8250_ioport + UART_MCR, 0x0B);
}
