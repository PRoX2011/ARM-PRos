#include <drivers/timer.h>

void timer_init(void) {
    /* System timer is free-running, no initialization required */
}

uint64_t timer_get_ticks(void) {
    uint32_t hi = TIMER_CHI;
    uint32_t lo = TIMER_CLO;
    
    if (hi != TIMER_CHI) {
        hi = TIMER_CHI;
        lo = TIMER_CLO;
    }
    
    return ((uint64_t)hi << 32) | lo;
}

void timer_sleep_us(uint32_t us) {
    uint64_t start = timer_get_ticks();
    while (timer_get_ticks() - start < us) {
        __asm__("nop");
    }
}

void timer_delay_ms(uint32_t ms) {
    timer_sleep_us(ms * 1000);
}
