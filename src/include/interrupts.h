#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

/* ARM Local Peripherals (BCM2837 QA7) */
#define LOCAL_PERIPHERAL_BASE   0x40000000u
#define CORE0_TIMER_IRQ_CTRL    ((volatile uint32_t *)(LOCAL_PERIPHERAL_BASE + 0x40u))
#define CORE0_IRQ_SOURCE        ((volatile uint32_t *)(LOCAL_PERIPHERAL_BASE + 0x60u))

/* IRQ Source bits */
#define IRQ_SOURCE_CNTPNSIRQ    (1u << 1) /* Non-secure Physical Timer IRQ */

void interrupts_init(void);
void enable_interrupts(void);
void disable_interrupts(void);

#endif /* INTERRUPTS_H */
