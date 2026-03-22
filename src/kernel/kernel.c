#include <drivers/console.h>
#include <drivers/framebuffer.h>
#include <drivers/uart.h>
#include <drivers/timer.h>
#include <drivers/usb.h>
#include <drivers/intc.h>
#include <kshell.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>

void c_irq_handler(void) {
    uint32_t irq = intc_get_pending();
    if (irq != 0xFFFFFFFF) {
        log_warn("Unhandled IRQ received!");
        intc_disable_irq(irq);  /* Disable to prevent infinite IRQ loop */
    }
}


const char *header = "=============================== ARM-PRos v0.1 ==============================\n\r";

const char *pros_logo = 
    "  _____  _____   ____   _____ \n\r"
    " |  __ \\|  __ \\ / __ \\ / ____|\n\r"
    " | |__) | |__) | |  | | (___  \n\r"
    " |  ___/|  _  /| |  | |\\___ \\ \n\r"
    " | |    | | \\ \\| |__| |____) |\n\r"
    " |_|    |_|  \\_\\\\____/|_____/ \n\r";

const char *copyright = "* Copyright (C) 2026 PRoX2011\n\r";
const char *shell = "* Shell: ARM-PRos kernel shell\n\r";

void main() {
    console_init();
    timer_init();
    intc_init();

    log_okay("UART PL011 serial console ready");
    log_okay("Interrupt Controller initialized");

    /* Enable IRQs globally (clear I bit in DAIF) */
    __asm__ volatile ("msr daifclr, #2");

    if (fb_is_ready())
        log_okay("Framebuffer 640x480, 32 bpp (VideoCore mailbox)");
    else
        log_warn("Framebuffer not available - HDMI output disabled");
    
    usb_init();
    log_okay("USB controller initialized");
    
    usb_enumerate_device();
    log_okay("USB device enumeration complete");
    
    log_okay("Kernel shell ready to start");

    console_puts("\n\rPress any key to continue...\n\r");
    (void)uart_getc();

    console_clear(0xFF202428u);

    console_puts(header);
    console_puts(pros_logo);
    console_puts("\n\r");
    console_puts(copyright);
    console_puts(shell);
    console_puts("\n\r");

    kshell_start();
}