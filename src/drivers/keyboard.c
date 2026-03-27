#include <drivers/keyboard.h>
#include <log.h>

#define KMI_BASE        0x09070000 

#define KMI_CR          (*(volatile unsigned int*)(KMI_BASE + 0x00))
#define KMI_STAT        (*(volatile unsigned int*)(KMI_BASE + 0x04))
#define KMI_DATA        (*(volatile unsigned int*)(KMI_BASE + 0x08))

#define KMI_CR_EN       (1 << 2)
#define KMI_STAT_RXFULL (1 << 4)

static const char ps2_set2_map[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '\t', '`', 0,
    0, 0, 0, 0, 0, 'q', '1', 0, 0, 0, 'z', 's', 'a', 'w', '2', 0,
    0, 'c', 'x', 'd', 'e', '4', '3', 0, 0, ' ', 'v', 'f', 't', 'r', '5', 0,
    0, 'n', 'b', 'h', 'g', 'y', '6', 0, 0, 0, 'm', 'j', 'u', '7', '8', 0,
    0, ',', 'k', 'i', 'o', '0', '9', 0, 0, '.', '/', 'l', ';', 'p', '-', 0,
    0, 0, '\'', 0, '[', '=', 0, 0, 0, 0, '\n', ']', 0, '\\', 0, 0,
    0, 0, 0, 0, 0, 0, '\b', 0, 0, '1', 0, '4', '7', 0, 0, 0,
    '0', '.', '2', '5', '6', '8', '\e', 0, 'f', '+', '3', '-', '*', '9', 0, 0
};

void keyboard_init(void) {
    KMI_CR |= KMI_CR_EN;
    log_okay("PL050 KMI Keyboard initialized");
}

unsigned char keyboard_get_scancode(void) {
    while ((KMI_STAT & KMI_STAT_RXFULL) == 0) {
    }
    return KMI_DATA & 0xFF;
}

char keyboard_getc(void) {
    unsigned char scancode;
    char ascii = 0;
    int is_break = 0;

    while (1) {
        scancode = keyboard_get_scancode();

        if (scancode == 0xF0) {
            is_break = 1;
            continue;
        }

        if (scancode == 0xE0) {
            continue;
        }

        if (!is_break) {
            if (scancode < sizeof(ps2_set2_map)) {
                ascii = ps2_set2_map[scancode];
                if (ascii != 0) {
                    return ascii;
                }
            }
        } else {
            is_break = 0; 
        }
    }
}
