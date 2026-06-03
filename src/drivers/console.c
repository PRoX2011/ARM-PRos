// ==================================================================
// ARM-PRos - unified text console (UART + framebuffer) for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/console.h>
#include <drivers/framebuffer.h>
#include <drivers/lcd/ili9486.h>
#include <drivers/uart.h>

static void uart_put_u8_dec(unsigned n)
{
	char tmp[4];
	int i = 0;

	if (n > 255u)
		n = 255u;
	if (n == 0u) {
		uart_putc('0');
		return;
	}
	while (n > 0u) {
		tmp[i++] = (char)('0' + (n % 10u));
		n /= 10u;
	}
	while (i > 0)
		uart_putc(tmp[--i]);
}

static void uart_clear_with_bg(uint32_t bg)
{
	unsigned r = (unsigned)((bg >> 16) & 0xFFu);
	unsigned g = (unsigned)((bg >> 8) & 0xFFu);
	unsigned b = (unsigned)(bg & 0xFFu);

	uart_puts("\033[48;2;");
	uart_put_u8_dec(r);
	uart_putc(';');
	uart_put_u8_dec(g);
	uart_putc(';');
	uart_put_u8_dec(b);
	uart_puts("m\033[2J\033[H");
}

void console_putc(char c)
{
	uart_putc(c);
	if (fb_is_ready())
		fb_putc((int)(unsigned char)c);
	if (lcd_is_ready())
		lcd_putc(c);
}

void console_puts(const char *s)
{
	uart_puts(s);
	if (fb_is_ready())
		fb_puts(s);
	if (lcd_is_ready())
		lcd_puts(s);
}

void console_clear(uint32_t bg)
{
	uart_clear_with_bg(bg);
	if (fb_is_ready()) {
		fb_set_bg(bg);
		fb_clear();
	}
	if (lcd_is_ready())
		lcd_clear(bg);
}
