// ==================================================================
// ARM-PRos - BCM2837 system timer driver for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/timer.h>

#define TIMER_BASE 0x3F003000u
#define TIMER_CLO  (*(volatile uint32_t *)(TIMER_BASE + 0x04u))
#define TIMER_CHI  (*(volatile uint32_t *)(TIMER_BASE + 0x08u))

uint64_t timer_get_ticks(void)
{
	uint32_t hi = TIMER_CHI;
	uint32_t lo = TIMER_CLO;
	if (TIMER_CHI != hi) {
		hi = TIMER_CHI;
		lo = TIMER_CLO;
	}
	return ((uint64_t)hi << 32) | lo;
}

void delay_us(uint32_t us)
{
	uint64_t target = timer_get_ticks() + us;
	while (timer_get_ticks() < target)
		__asm__ volatile("yield");
}

void delay_ms(uint32_t ms)
{
	delay_us(ms * 1000u);
}
