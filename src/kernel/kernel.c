#include <drivers/uart.h>

const char *header = "================================ ARM-PRos v0.1 ================================\n\r";

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
    uart_puts(header);
    uart_puts(pros_logo);
    uart_puts("\n\r");
    uart_puts(copyright);
    uart_puts(shell);

    while (1) {
        __asm__("wfe");
    }
}