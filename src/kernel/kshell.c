#include <kshell.h>
#include <drivers/console.h>
#include <drivers/uart.h>
#include <log.h>
#include <string.h>
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

            if (cmd_len > 0) {
                if (strcmp(cmd_buffer, "help") == 0) {
                    console_puts("Available commands:\n\r");
                    console_puts("  help    - Show this help message\n\r");
                    console_puts("  cls     - Clear screen\n\r");
                    console_puts("  sysinfo - Show system information\n\r");
                    console_puts("  echo    - Print text\n\r");
                    console_puts("  add     - Add two numbers\n\r");
                }
                else if (strcmp(cmd_buffer, "cls") == 0) {
                    console_clear(0xFF202428u);
                }
                else if (strcmp(cmd_buffer, "sysinfo") == 0) {
                    console_puts("OS: ARM-PRos v0.1\n\r");
                    console_puts("Resolution: 640x480, 32 bpp\n\r");
                    console_puts("Build target: Raspberry Pi 2/3\n\r");
                }
                else if (strncmp(cmd_buffer, "echo ", 5) == 0) {
                    console_puts(&cmd_buffer[5]);
                    console_puts("\n\r");
                }
                else if (strncmp(cmd_buffer, "add ", 4) == 0) {
                    int i = 4;
                    int num1 = atoi(&cmd_buffer[i]);
                    
                    while (cmd_buffer[i] != ' ' && cmd_buffer[i] != '\0') i++;
                    while (cmd_buffer[i] == ' ' && cmd_buffer[i] != '\0') i++;
                    
                    int num2 = atoi(&cmd_buffer[i]);
                    int sum = num1 + num2;
                    
                    if (sum < 0) {
                        console_putc('-');
                        sum = -sum;
                    }
                    
                    if (sum == 0) {
                        console_putc('0');
                    } else {
                        char res[16];
                        int pos = 0;
                        while (sum > 0) {
                            res[pos++] = (char)('0' + (sum % 10));
                            sum /= 10;
                        }
                        while (pos > 0) {
                            console_putc(res[--pos]);
                        }
                    }
                    console_puts("\n\r");
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
