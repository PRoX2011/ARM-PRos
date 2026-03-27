#include <drivers/console.h>
#include <drivers/framebuffer.h>
#include <drivers/uart.h>
#include <drivers/keyboard.h>
#include <drivers/sd.h>
#include <kshell.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>

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

    log_okay("UART PL011 serial console ready");
    if (fb_is_ready())
        log_okay("Framebuffer 640x480, 32 bpp (VideoCore mailbox)");
    else
        log_warn("Framebuffer not available - HDMI output disabled");

    keyboard_init();

    if (sd_init()) {
        log_okay("SD Card (EMMC) initialized");
    } else {
        log_error("SD Card initialization failed");
    }
    
    log_okay("Kernel shell ready to start");

    console_puts("\n\rPress any key on UART OR Keyboard to continue...\n\r");
    
    (void)keyboard_getc();

    console_clear(0xFF202428u);

    console_puts(header);
    console_puts(pros_logo);
    console_puts("\n\r");
    console_puts(copyright);
    console_puts(shell);
    console_puts("\n\r");

    kshell_start();
}
