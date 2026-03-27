#include <kshell.h>
#include <drivers/console.h>
#include <drivers/uart.h>
#include <drivers/sd.h>
#include <log.h>
#include <string.h>

#define CMD_MAX_LEN 256

void kshell_start(void) {
    char cmd_buffer[CMD_MAX_LEN];
    int cmd_len = 0;

    console_puts("\n\r[PRos] > ");

    while (1) {
        char c = uart_getc();

        if (c == '\r' || c == '\n') {
            console_puts("\n\r");
            cmd_buffer[cmd_len] = '\0';

            if (cmd_len > 0) {
                if (strcmp(cmd_buffer, "help") == 0) {
                    console_puts("Available commands:\n\r");
                    console_puts("  help   - Show this help message\n\r");
                    console_puts("  cls    - Clear screen\n\r");
                    console_puts("  sdtest - Test SD card sector 0\n\r");
                }
                else if (strcmp(cmd_buffer, "cls") == 0) {
                    console_clear(0xFF202428u);
                }
                else if (strcmp(cmd_buffer, "sdtest") == 0) {
                    uint8_t buf[512];
                    if (sd_read_sector(0, buf)) {
                        console_puts("Sector 0 read successfully!\n\r");
                    } else {
                        log_error("Failed to read SD sector");
                    }
                }
                else {
                    char msg[CMD_MAX_LEN + 32];
                    unsigned mi = 0u;
                    const char *pre = "Unknown command: ";

                    while (*pre != '\0' && mi < sizeof(msg) - 1u)
                        msg[mi++] = *pre++;
                    unsigned pj = 0u;
                    while (cmd_buffer[pj] != '\0' &&
                           mi < sizeof(msg) - 1u)
                        msg[mi++] = cmd_buffer[pj++];
                    msg[mi] = '\0';
                    log_error(msg);
                }
            }

            cmd_len = 0;
            console_puts("[PRos] > ");
        } 
        else if (c == '\b' || c == 0x7F) {
            if (cmd_len > 0) {
                cmd_len--;
                console_puts("\b \b");
            }
        } 
        else if (c >= 32 && c <= 126) {
            if (cmd_len < CMD_MAX_LEN - 1) {
                cmd_buffer[cmd_len++] = c;
                console_putc(c);
            }
        }
    }
}
