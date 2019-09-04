#include <kernel/printf.h>

#include <kernel/console.h>

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef void (*fmt_putchar_t)(int c, void *priv);

/// An enum type.
/// Enum desribes the format flags for prints method.

enum format_flags {
	FMT_LONG = 1UL << 0,
	FMT_LONG_LONG = 1UL << 1,
	FMT_ZERO_PAD = 1UL << 2,
	FMT_LEFT = 1UL << 3,
	FMT_UNSIGNED = 1UL << 4,
	FMT_PRECISION = 1UL << 5,
	FMT_WIDTH = 1UL << 6,
};

/// \struct format_state
/// \brief  Flags and width for the format state.

struct format_state {
	enum format_flags flags;
	size_t width;
};

static const char number_chars_upcase[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
};

static const char number_chars[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};


/// format_number formats a number per a particular base.
/// \param buf   buffer pointer
/// \param n     maximum size of string
/// \param len   actual length of string
/// \param chars characters passed from input
/// \param base  base of the output
/// \param sign  signed or unsigned
/// \return      formatter number

static char *format_number(char *buf, uint64_t n, size_t len, const char *chars, uint64_t base, bool sign)
{
	bool negative = (int64_t)n < 0;
	if (sign && negative) {
		n = -n;
	}
	size_t pos = len;
	buf[--pos] = 0;
	while (n >= base) {
		int digit = n % base;
		n /= base;
		buf[--pos] = chars[digit];
	}
	buf[--pos] = chars[n];
	if (sign && negative) {
		buf[--pos] = '-';
	}
	return &buf[pos];
}

static void format_str(const char *buf, size_t len, struct format_state *fmt, fmt_putchar_t putchar, void *priv)
{
	size_t length = len;
	size_t padding = 0;
	if (fmt->flags & FMT_WIDTH) {
		if (length > fmt->width) {
			length = fmt->width;
		}
		padding = fmt->width - length;
	}
	if (fmt->flags & FMT_LEFT) {
		while (length > 0) {
			char c = *buf++;
			if (!c) {
				break;
			}
			putchar(c, priv);
			length--;
		}
		while (padding > 0) {
			putchar(' ', priv);
			padding--;
		}
	} else {
		char pad = fmt->flags & FMT_ZERO_PAD ? '0' : ' ';
		while (padding > 0) {
			putchar(pad, priv);
			padding--;
		}
		while (length > 0) {
			char c = *buf++;
			if (!c) {
				break;
			}
			putchar(c, priv);
			length--;
		}
	}
}

static uint64_t pop_int(va_list ap, struct format_state *fmt)
{
	if (fmt->flags & FMT_UNSIGNED) {
		if (fmt->flags & FMT_LONG_LONG) {
			return va_arg(ap, unsigned long long);
		} else if (fmt->flags & FMT_LONG) {
			return va_arg(ap, unsigned long);
		} else {
			return va_arg(ap, unsigned int);
		}
	} else {
		if (fmt->flags & FMT_LONG_LONG) {
			return va_arg(ap, long long);
		}
		if (fmt->flags & FMT_LONG) {
			return va_arg(ap, long);
		}
		return va_arg(ap, int);
	}
}

static const char *format(const char *fmt, va_list ap, fmt_putchar_t putchar, void *priv)
{
	struct format_state fmt_state = {
		.flags = 0,
		.width = 0,
	};
	for (;;) {
		int c = *fmt++;
		switch (c) {
		/* Field width and precision: */
		case '0' ... '9': {
			/* Precision is ignored */
			if (!(fmt_state.flags & FMT_PRECISION)) {
				fmt_state.flags |= FMT_WIDTH;
				if (fmt_state.width == 0 && c == '0') {
					fmt_state.flags |= FMT_ZERO_PAD;
				}
				fmt_state.width *= 10;
				fmt_state.width += c - '0';
			}
			break;
		}
		case '.': {
			fmt_state.flags |= FMT_PRECISION;
			fmt_state.flags |= FMT_ZERO_PAD;
			break;
		}
		/* Flags: */
		case '-': {
			fmt_state.flags |= FMT_LEFT;
			break;
		}
		case 'l': {
			if (fmt_state.flags & FMT_LONG) {
				fmt_state.flags |= FMT_LONG_LONG;
			}
			fmt_state.flags |= FMT_LONG;
			break;
		}
		/* Conversion specifiers: */
		case 'd': {
			char buffer[32];
			uint64_t d = pop_int(ap, &fmt_state);
			const char *s = format_number(buffer, d, ARRAY_SIZE(buffer), number_chars, 10, true);
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case 'u': {
			fmt_state.flags |= FMT_UNSIGNED;
			char buffer[32];
			uint64_t d = pop_int(ap, &fmt_state);
			const char *s = format_number(buffer, d, ARRAY_SIZE(buffer), number_chars, 10, false);
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case 'x': {
			fmt_state.flags |= FMT_UNSIGNED;
			char buffer[32];
			uint64_t d = pop_int(ap, &fmt_state);
			const char *s = format_number(buffer, d, ARRAY_SIZE(buffer), number_chars, 16, false);
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case 'X': {
			fmt_state.flags |= FMT_UNSIGNED;
			char buffer[32];
			uint64_t d = pop_int(ap, &fmt_state);
			const char *s = format_number(buffer, d, ARRAY_SIZE(buffer), number_chars_upcase, 16, false);
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case 's': {
			const char *s = va_arg(ap, const char *);
			if (!s) {
				s = "<null>";
			}
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case 'p': {
			putchar('0', priv);
			putchar('x', priv);
			fmt_state.flags |= FMT_UNSIGNED;
			char buffer[32];
			void *p = va_arg(ap, void *);
			const char *s =
			    format_number(buffer, (unsigned long)p, ARRAY_SIZE(buffer), number_chars, 16, false);
			format_str(s, strlen(s), &fmt_state, putchar, priv);
			goto out;
		}
		case '%': {
			putchar('%', priv);
			goto out;
		}
		/* Unknown format specifier: */
		default:
			putchar('%', priv);
			putchar(c, priv);
			goto out;
		}
	}
out:
	return fmt;
}

struct vprintf_state {
	size_t pos;
};

static void vprintf_output(int c, void *priv)
{
	struct vprintf_state *state = priv;
	putchar(c);
	state->pos++;
}

int vprintf(const char *fmt, va_list ap)
{
	struct vprintf_state state = {
	    .pos = 0,
	};
	for (;;) {
		char c = *fmt++;
		if (c == '\0') {
			break;
		}
		if (c != '%') {
			putchar(c);
			continue;
		}
		fmt = format(fmt, ap, vprintf_output, &state);
	}
	return state.pos;
}

int printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vprintf(fmt, ap);
	va_end(ap);
	return ret;
}

int putchar(int c)
{
	console_write_char(c);
	return c;
}

int puts(const char *s)
{
	console_write_str(s);
	return 1;
}

struct vsprintf_state {
	char *str;
	size_t pos;
	size_t size;
};

static void vsprintf_output(int c, void *priv)
{
	struct vsprintf_state *state = priv;
	if (state->pos < state->size) {
		state->str[state->pos] = c;
	}
	state->pos++;
}

int vsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	struct vsprintf_state state = {
	    .str = str,
	    .pos = 0,
	    .size = size,
	};
	for (;;) {
		char c = *fmt++;
		if (c == '\0') {
			break;
		}
		if (c != '%') {
			vsprintf_output(c, &state);
			continue;
		}
		fmt = format(fmt, ap, vsprintf_output, &state);
	}
	if (state.pos > state.size) {
		str[state.size - 1] = '\0';
	} else {
		str[state.pos] = '\0';
	}
	return state.pos;
}

int vsprintf(char *str, const char *fmt, va_list ap)
{
	return vsnprintf(str, SIZE_MAX, fmt, ap);
}

int sprintf(char *str, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vsprintf(str, fmt, ap);
	va_end(ap);
	return ret;
}

int snprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int ret = vsnprintf(str, size, fmt, ap);
	va_end(ap);
	return ret;
}
