#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

void console_init(void);
void console_write_char(char ch);
void console_write(const char *s);

#endif
