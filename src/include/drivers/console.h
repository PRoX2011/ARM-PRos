#ifndef DRIVERS_CONSOLE_H
#define DRIVERS_CONSOLE_H

#include <stdint.h>

void console_putc(char c);
void console_puts(const char *s);
void console_clear(uint32_t bg);

#endif
