#include <timer.h>
#include <interrupts.h>
#include <drivers/console.h>

static uint64_t system_ticks = 0;
static uint32_t timer_interval = 0;

static inline uint64_t read_cntfrq(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}

static inline void write_cntp_tval(uint64_t val) {
    __asm__ volatile("msr cntp_tval_el0, %0" : : "r"(val));
}

static inline void write_cntp_ctl(uint64_t val) {
    __asm__ volatile("msr cntp_ctl_el0, %0" : : "r"(val));
}

void timer_init(void) {
    uint64_t freq = read_cntfrq();
    timer_interval = (uint32_t)(freq / 100);
    
    write_cntp_tval(timer_interval);
    write_cntp_ctl(1);
}

void timer_irq_handler(void) {
    system_ticks++;
    write_cntp_tval(timer_interval);
}

uint64_t timer_get_ticks(void) {
    return system_ticks;
}

void timer_sleep(uint64_t ms) {
    uint64_t start = system_ticks;
    uint64_t ticks_to_wait = ms / 10;
    while (system_ticks - start < ticks_to_wait) {
        __asm__ volatile("wfe");
    }
}
