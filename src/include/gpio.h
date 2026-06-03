#ifndef GPIO_H
#define GPIO_H

#define MMIO_BASE       0x3F000000u

// ------ Function select (3 bits per pin, 10 pins per register) ------
#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))

// ------ Output set / clear, pin level ------
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))

// ------ Event detect / high-edge enable ------
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))

// ------ Pull up/down control ------
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

// ------ Pin function codes (the 3-bit FSEL value) ------
#define GPIO_INPUT      0u
#define GPIO_OUTPUT     1u
#define GPIO_ALT0       4u

static inline void gpio_set_function(unsigned pin, unsigned func)
{
	volatile unsigned int *reg = GPFSEL0 + (pin / 10u);
	unsigned shift = (pin % 10u) * 3u;
	*reg = (*reg & ~(7u << shift)) | ((func & 7u) << shift);
}

#endif