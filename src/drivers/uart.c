// ==================================================================
// ARM-PRos - PL011 UART driver for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/uart.h>

#define PERIPHERAL_BASE     0x3F000000u
#define GPIO_BASE           (PERIPHERAL_BASE + 0x200000u)
#define UART0_BASE          (PERIPHERAL_BASE + 0x201000u)

#define GPFSEL1             (*(volatile unsigned int *)(GPIO_BASE + 0x04u))
#define UART0_DR            (*(volatile unsigned int *)(UART0_BASE + 0x00u))
#define UART0_FR            (*(volatile unsigned int *)(UART0_BASE + 0x18u))
#define UART0_IBRD          (*(volatile unsigned int *)(UART0_BASE + 0x24u))
#define UART0_FBRD          (*(volatile unsigned int *)(UART0_BASE + 0x28u))
#define UART0_LCRH          (*(volatile unsigned int *)(UART0_BASE + 0x2Cu))
#define UART0_CR            (*(volatile unsigned int *)(UART0_BASE + 0x30u))
#define UART0_ICR           (*(volatile unsigned int *)(UART0_BASE + 0x44u))

#define UART_CLOCK_HZ       48000000u
#define UART_BAUD           115200u

static void gpio_uart_pins_alt0(void)
{
	unsigned int r = GPFSEL1;
	r &= ~(7u << 12);
	r |=  (4u << 12);
	r &= ~(7u << 15);
	r |=  (4u << 15);
	GPFSEL1 = r;
}

void uart_init(void)
{
	gpio_uart_pins_alt0();

	UART0_CR = 0u;
	UART0_ICR = 0x7FFu;

	unsigned long long denom = 16ull * UART_BAUD;
	unsigned long long div64 =
	    (UART_CLOCK_HZ * 64ull + denom / 2ull) / denom;
	UART0_IBRD = (unsigned int)(div64 / 64ull);
	UART0_FBRD = (unsigned int)(div64 % 64ull);

	UART0_LCRH = (1u << 4) | (3u << 5);
	UART0_CR   = (1u << 0) | (1u << 8) | (1u << 9);
}

void uart_putc(char c)
{
	while (UART0_FR & (1u << 5)) { }
	UART0_DR = (unsigned int)(unsigned char)c;
}

void uart_puts(const char *s)
{
	for (int i = 0; s[i] != '\0'; i++)
		uart_putc(s[i]);
}

char uart_getc(void)
{
	while (UART0_FR & (1u << 4)) { }
	return (char)(UART0_DR & 0xFFu);
}