#include <kshell.h>
#include <drivers/console.h>
#include <drivers/uart.h>
#include <log.h>
#include <string.h>
#include <power.h>
#include <timer.h>
#include <stdlib.h>

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

            char *cmd = cmd_buffer;
            while (*cmd == ' ') cmd++;
            
            char *end = cmd + strlen(cmd);
            while (end > cmd && *(end - 1) == ' ') end--;
            *end = '\0';

            if (strlen(cmd) > 0) {
                if (strcmp(cmd, "help") == 0) {
                    console_puts("Available commands:\n\r");
                    console_puts("  help   - Show this help message\n\r");
                    console_puts("  cls    - Clear screen\n\r");
                    console_puts("  ticks  - Show system ticks\n\r");
                    console_puts("  shut   - Shut down your device\n\r");
                    console_puts("  reboot - Reboot down your device\n\r");
                }
                else if (strcmp(cmd, "cls") == 0) {
                    console_clear(0xFF202428u);
                }
                else if (strcmp(cmd, "ticks") == 0) {
                    char buf[32];
                    console_puts("System ticks: ");
                    console_puts(itoa((int)timer_get_ticks(), buf, 10));
                    console_puts("\n\r");
                }
                else if (strcmp(cmd, "shut") == 0) {
                    power_off();
                }
                else if (strcmp(cmd, "reboot") == 0) {
                    reboot();
                }
                else {
                    char msg[CMD_MAX_LEN + 32];
                    unsigned mi = 0u;
                    const char *pre = "Unknown command: ";

                    while (*pre != '\0' && mi < sizeof(msg) - 1u)
                        msg[mi++] = *pre++;
                    unsigned pj = 0u;
                    while (cmd[pj] != '\0' &&
                           mi < sizeof(msg) - 1u)
                        msg[mi++] = cmd[pj++];
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