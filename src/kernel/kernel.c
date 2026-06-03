// ==================================================================
// ARM-PRos - kernel entry point for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/console.h>
#include <drivers/framebuffer.h>
#include <drivers/lcd/ili9486.h>
#include <drivers/uart.h>
#include <drivers/input.h>
#include <drivers/usb/keyboard.h>
#include <kernel/boot_menu.h>
#include <kshell.h>
#include <log.h>
#include <string.h>
#include <stdlib.h>
#include <interrupts.h>
#include <timer.h>
#include <pmm.h>
#include <malloc.h>

const char *header = "=============================== ARM-PRos v0.1 ==============================\n\r";

const char *pros_logo =
    "  _____  _____   ____   _____ \n\r"
    " |  __ \\|  __ \\ / __ \\ / ____|\n\r"
    " | |__) | |__) | |  | | (___  \n\r"
    " |  ___/|  _  /| |  | |\\___ \\ \n\r"
    " | |    | | \\ \\| |__| |____) |\n\r"
    " |_|    |_|  \\_\\\\____/|_____/ \n\r";

const char *copyright = "* Copyright (C) 2026 PRoX2011\n\r";
const char *shell_str = "* Shell: ARM-PRos kernel shell\n\r";

void main()
{
	uart_init();
	fb_init(640, 480, 32u);
	fb_set_font_height(8);
	fb_clear();
	lcd_init();
	input_init();

	if (usb_kbd_available())
		log_okay("USB keyboard detected");
	else
		log_warn("No USB keyboard. Using UART input");

	boot_menu_result_t bmr;
	boot_menu_run(&bmr);

	fb_init(bmr.width, bmr.height, 32u);
	fb_set_font_height(bmr.font_h);
	fb_clear();

	lcd_set_font_height(bmr.font_h);
	lcd_clear(0x202428u);

	log_okay("UART PL011 serial console ready");

	if (fb_is_ready())
		log_okay("Framebuffer ready");
	else
		log_warn("Framebuffer not available. HDMI output disabled. Using UART");

	if (lcd_is_ready())
		log_okay("ILI9486 SPI LCD ready");
	else
		log_warn("ILI9486 SPI LCD not available");

	if (usb_kbd_available())
		log_okay("USB keyboard ready");

	log_okay("Kernel shell ready to start :)");

    log_okay("UART PL011 serial console ready");
    if (fb_is_ready())
        log_okay("Framebuffer 640x480, 32 bpp (VideoCore mailbox)");
    else
        log_warn("Framebuffer not available - HDMI output disabled");
    
    interrupts_init();
    timer_init();
    pmm_init();
    malloc_init();
    enable_interrupts();

    log_okay("Memory, Interrupts and Generic Timer initialized");
    log_okay("Kernel shell ready to start");

	console_clear(0xFF202428u);

	const char *header;
    if (bmr.use_spi) {
	    header = "============ ARM-PRos v0.1 ============\n\r";
    }
    else {
        header = "=============================== ARM-PRos v0.1 ==============================\n\r";
    }

	console_puts(header);
	console_puts(pros_logo);
	console_puts("\n\r");
	console_puts(copyright);
	console_puts(shell_str);
	console_puts("\n\r");

	kshell_start();
}