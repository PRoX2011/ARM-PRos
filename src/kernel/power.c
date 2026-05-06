#include <stdint.h>
#include <gpio.h>
#include <drivers/mailbox.h>

#define PM_RSTC         ((volatile uint32_t *)(MMIO_BASE + 0x0010001c))
#define PM_RSTS         ((volatile uint32_t *)(MMIO_BASE + 0x00100020))
#define PM_WDOG         ((volatile uint32_t *)(MMIO_BASE + 0x00100024))
#define PM_WDOG_MAGIC   0x5a000000u
#define PM_RSTC_FULLRST 0x00000020u

#define MBOX_REQUEST      0x00000000u
#define MBOX_TAG_SETPOWER 0x00028001u
#define MBOX_TAG_LAST     0x00000000u

static void wait_cycles(unsigned int n)
{
	while (n--) {
		__asm__ volatile("nop");
	}
}

void power_off(void)
{
	/* Turn off all devices via mailbox */
	for (unsigned int r = 0; r < 16; r++) {
		volatile uint32_t __attribute__((aligned(16))) msg[8];
		msg[0] = 8 * 4;
		msg[1] = MBOX_REQUEST;
		msg[2] = MBOX_TAG_SETPOWER;
		msg[3] = 8;
		msg[4] = 8;
		msg[5] = r;
		msg[6] = 0;
		msg[7] = MBOX_TAG_LAST;
		mbox_call(msg);
	}

	/* Reset all GPIO pins */
	*GPFSEL0 = 0; *GPFSEL1 = 0; *GPFSEL2 = 0;
	*GPFSEL3 = 0; *GPFSEL4 = 0; *GPFSEL5 = 0;

	*GPPUD = 0;
	wait_cycles(150);
	*GPPUDCLK0 = 0xffffffff;
	*GPPUDCLK1 = 0xffffffff;
	wait_cycles(150);
	*GPPUDCLK0 = 0;
	*GPPUDCLK1 = 0;

	uint32_t val = *PM_RSTS;
	val &= ~0xfffffaaau;
	val |= 0x555u;
	*PM_RSTS = PM_WDOG_MAGIC | val;
	*PM_WDOG = PM_WDOG_MAGIC | 10;
	*PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}

void reboot(void)
{
	uint32_t val = *PM_RSTS;
	val &= ~0xfffffaaau;
	*PM_RSTS = PM_WDOG_MAGIC | val;
	*PM_WDOG = PM_WDOG_MAGIC | 10;
	*PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}
