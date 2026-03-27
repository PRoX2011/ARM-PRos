#include <drivers/sd.h>
#include <drivers/mailbox.h>
#include <stdint.h>

#define EMMC_BASE       0x3F300000u
#define EMMC_ARG2       (*(volatile uint32_t*)(EMMC_BASE + 0x00))
#define EMMC_BLKSIZECNT (*(volatile uint32_t*)(EMMC_BASE + 0x04))
#define EMMC_ARG1       (*(volatile uint32_t*)(EMMC_BASE + 0x08))
#define EMMC_CMDTM      (*(volatile uint32_t*)(EMMC_BASE + 0x0C))
#define EMMC_RESP0      (*(volatile uint32_t*)(EMMC_BASE + 0x10))
#define EMMC_DATA       (*(volatile uint32_t*)(EMMC_BASE + 0x20))
#define EMMC_STATUS     (*(volatile uint32_t*)(EMMC_BASE + 0x24))
#define EMMC_CONTROL0   (*(volatile uint32_t*)(EMMC_BASE + 0x28))
#define EMMC_CONTROL1   (*(volatile uint32_t*)(EMMC_BASE + 0x2C))
#define EMMC_INTERRUPT  (*(volatile uint32_t*)(EMMC_BASE + 0x30))
#define EMMC_IRPT_MASK  (*(volatile uint32_t*)(EMMC_BASE + 0x34))
#define EMMC_IRPT_EN    (*(volatile uint32_t*)(EMMC_BASE + 0x38))
#define EMMC_CONTROL2   (*(volatile uint32_t*)(EMMC_BASE + 0x3C))
#define EMMC_SLOTISR_VER (*(volatile uint32_t*)(EMMC_BASE + 0xFC))

#define GPIO_BASE       0x3F200000u
#define GPPUD           (*(volatile uint32_t*)(GPIO_BASE + 0x94))
#define GPPUDCLK1       (*(volatile uint32_t*)(GPIO_BASE + 0x9C))
#define GPFSEL4         (*(volatile uint32_t*)(GPIO_BASE + 0x10))
#define GPFSEL5         (*(volatile uint32_t*)(GPIO_BASE + 0x14))

static uint32_t sd_rca;

static void delay(int n) {
    while (n--) { __asm__ volatile("nop"); }
}

static int sd_status(uint32_t mask) {
    int timeout = 1000000;
    while (!(EMMC_INTERRUPT & mask) && timeout--) {
        delay(1);
    }
    if (timeout <= 0) return 0;
    EMMC_INTERRUPT = mask;
    return 1;
}

static int sd_cmd(uint32_t cmd, uint32_t arg) {
    uint32_t code = (cmd << 24);
    if (cmd == 17 || cmd == 18 || cmd == 24 || cmd == 25) code |= (1 << 21) | (1 << 4);
    if (cmd == 8 || cmd == 13 || cmd == 1) code |= (1 << 16);
    if (cmd == 2 || cmd == 9) code |= (1 << 16) | (1 << 17);
    while (EMMC_STATUS & 0x01) delay(1);
    EMMC_ARG1 = arg;
    EMMC_CMDTM = code;
    if (!sd_status(0x01)) return 0;
    return 1;
}

int sd_init(void) {
    uint32_t r = GPFSEL4;
    r &= ~(7 << 24); r |= (7 << 24); 
    r &= ~(7 << 27); r |= (7 << 27); 
    GPFSEL4 = r;
    r = GPFSEL5;
    r &= ~0x3FFFF; r |= 0x24924;
    GPFSEL5 = r;
    GPPUD = 0; delay(150);
    GPPUDCLK1 = (1 << 16) | (1 << 17) | (1 << 18) | (1 << 19) | (1 << 20) | (1 << 21);
    delay(150); GPPUDCLK1 = 0;
    EMMC_CONTROL1 = (1 << 24);
    while (EMMC_CONTROL1 & (1 << 24));
    EMMC_CONTROL1 |= (1 << 0) | (7 << 16);
    delay(100);
    EMMC_CONTROL1 |= (1 << 2);
    while (!(EMMC_CONTROL1 & (1 << 1)));
    EMMC_IRPT_MASK = 0xFFFFFFFF;
    EMMC_IRPT_EN = 0xFFFFFFFF;
    sd_cmd(0, 0);
    sd_cmd(8, 0x1AA);
    uint32_t ocr = 0;
    for (int i = 0; i < 1000; i++) {
        sd_cmd(55, 0);
        sd_cmd(41, 0x40100000);
        ocr = EMMC_RESP0;
        if (ocr & (1 << 31)) break;
        delay(1000);
    }
    sd_cmd(2, 0);
    sd_cmd(3, 0);
    sd_rca = EMMC_RESP0 & 0xFFFF0000;
    sd_cmd(7, sd_rca);
    EMMC_BLKSIZECNT = (1 << 16) | 512;
    return 1;
}

int sd_read_sector(uint32_t lba, uint8_t *buffer) {
    EMMC_BLKSIZECNT = (1 << 16) | 512;
    if (!sd_cmd(17, lba)) return 0;
    if (!sd_status(0x20)) return 0;
    uint32_t *ptr = (uint32_t*)buffer;
    for (int i = 0; i < 128; i++) ptr[i] = EMMC_DATA;
    return 1;
}

int sd_write_sector(uint32_t lba, uint8_t *buffer) {
    EMMC_BLKSIZECNT = (1 << 16) | 512;
    if (!sd_cmd(24, lba)) return 0;
    if (!sd_status(0x10)) return 0;
    uint32_t *ptr = (uint32_t*)buffer;
    for (int i = 0; i < 128; i++) EMMC_DATA = ptr[i];
    if (!sd_status(0x02)) return 0;
    return 1;
}
