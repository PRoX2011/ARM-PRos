#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <stdint.h>

uint64_t timer_get_ticks(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif
