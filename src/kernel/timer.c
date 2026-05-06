#include <timer.h>
#include <interrupts.h>
#include <drivers/console.h>

static uint64_t system_ticks = 0;
static uint32_t timer_interval = 0;

/* System Registers Access */
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
    /* Set interval to 100Hz (10ms) */
    timer_interval = (uint32_t)(freq / 100);
    
    write_cntp_tval(timer_interval);
    write_cntp_ctl(1); /* Enable timer, unmask interrupt */
}

void timer_irq_handler(void) {
    system_ticks++;
    
    /* Reset timer for next interrupt */
    write_cntp_tval(timer_interval);

    /* Visual confirmation of tick (optional/debug) */
    if (system_ticks % 100 == 0) {
        // console_puts("."); // Commented out to not clutter the shell too much
    }
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
