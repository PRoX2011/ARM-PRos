#include <interrupts.h>
#include <log.h>
#include <drivers/console.h>
#include <drivers/uart.h>
#include <stdlib.h>

/* Defined in timer.c */
void timer_irq_handler(void);

void handle_invalid_exception(int type) {
    char buf[32];
    log_error("Invalid exception occurred!");
    console_puts("Type: ");
    console_puts(itoa(type, buf, 10));
    console_puts("\n\r");
    while(1);
}

void handle_el1_sync(void) {
    uint64_t esr, far, elr;
    __asm__ volatile("mrs %0, esr_el1" : "=r"(esr));
    __asm__ volatile("mrs %0, far_el1" : "=r"(far));
    __asm__ volatile("mrs %0, elr_el1" : "=r"(elr));

    log_error("EL1 Synchronous exception!");
    
    char buf[64];
    console_puts("ESR_EL1: ");
    uart_puthex(esr);
    console_puts("\n\rFAR_EL1: ");
    uart_puthex(far);
    console_puts("\n\rELR_EL1: ");
    uart_puthex(elr);
    console_puts("\n\r");

    while(1);
}

void handle_el1_irq(void) {
    uint32_t source = *CORE0_IRQ_SOURCE;

    if (source & IRQ_SOURCE_CNTPNSIRQ) {
        timer_irq_handler();
    } else {
        log_warn("Unknown IRQ source!");
    }
}

void interrupts_init(void) {
    /* 
     * In a multi-core environment, this would need to be done for each core.
     * For now, we only care about Core 0.
     */
    *CORE0_TIMER_IRQ_CTRL = IRQ_SOURCE_CNTPNSIRQ;
}
