#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <stdint.h>

/*
 BCM2837 System Timer
*/

#define TIMER_BASE              0x3F003000

#define TIMER_CS                (*(volatile uint32_t*)(TIMER_BASE + 0x00))
#define TIMER_CLO               (*(volatile uint32_t*)(TIMER_BASE + 0x04))
#define TIMER_CHI               (*(volatile uint32_t*)(TIMER_BASE + 0x08))
#define TIMER_C0                (*(volatile uint32_t*)(TIMER_BASE + 0x0C))
#define TIMER_C1                (*(volatile uint32_t*)(TIMER_BASE + 0x10))
#define TIMER_C2                (*(volatile uint32_t*)(TIMER_BASE + 0x14))
#define TIMER_C3                (*(volatile uint32_t*)(TIMER_BASE + 0x18))

void timer_init(void);
uint64_t timer_get_ticks(void);

void timer_sleep_us(uint32_t us);
void timer_delay_ms(uint32_t ms);

#endif
