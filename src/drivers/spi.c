// ==================================================================
// ARM-PRos - BCM2837 SPI0 master driver for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/spi.h>
#include <gpio.h>

#define SPI0_BASE 0x3F204000u
#define SPI0_CS   (*(volatile uint32_t *)(SPI0_BASE + 0x00u))
#define SPI0_FIFO (*(volatile uint32_t *)(SPI0_BASE + 0x04u))
#define SPI0_CLK  (*(volatile uint32_t *)(SPI0_BASE + 0x08u))

#define CS_CLEAR (3u << 4)
#define CS_TA    (1u << 7)
#define CS_DONE  (1u << 16)
#define CS_RXD   (1u << 17)
#define CS_TXD   (1u << 18)

void spi0_init(uint32_t clock_divider)
{
	gpio_set_function(8, GPIO_ALT0);  /* CE0  */
	gpio_set_function(9, GPIO_ALT0);  /* MISO */
	gpio_set_function(10, GPIO_ALT0); /* MOSI */
	gpio_set_function(11, GPIO_ALT0); /* SCLK */

	SPI0_CLK = clock_divider;
	SPI0_CS = CS_CLEAR;
}

void spi0_write(const uint8_t *data, size_t len)
{
	SPI0_CS = CS_CLEAR | CS_TA;

	for (size_t i = 0; i < len; i++) {
		while (!(SPI0_CS & CS_TXD))
			;
		SPI0_FIFO = data[i];
		while (SPI0_CS & CS_RXD)
			(void)SPI0_FIFO;
	}

	while (!(SPI0_CS & CS_DONE)) {
		while (SPI0_CS & CS_RXD)
			(void)SPI0_FIFO;
	}

	SPI0_CS &= ~CS_TA;
}
