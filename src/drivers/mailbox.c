// ==================================================================
// ARM-PRos - VideoCore mailbox property interface for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/mailbox.h>
#include <stdint.h>

#define PERIPHERAL_BASE 0x3F000000u
#define MBOX_BASE       (PERIPHERAL_BASE + 0xB880u)
#define MBOX_READ       (*(volatile uint32_t *)(MBOX_BASE + 0x00u))
#define MBOX_STATUS     (*(volatile uint32_t *)(MBOX_BASE + 0x18u))
#define MBOX_WRITE      (*(volatile uint32_t *)(MBOX_BASE + 0x20u))

#define MBOX_FULL       0x80000000u
#define MBOX_EMPTY      0x40000000u
#define MBOX_CH_PROP    8u

#define GPU_MEM_BASE    0x40000000u

static inline void dmb_sy(void)
{
	__asm__ volatile("dmb sy" ::: "memory");
}

static uint32_t arm_ptr_to_bus(volatile void *p)
{
	uintptr_t phys = (uintptr_t)p;
	return (uint32_t)(phys | GPU_MEM_BASE);
}

int mbox_call(volatile uint32_t *msg)
{
	uint32_t bus = (arm_ptr_to_bus(msg) & ~0xFu) | (MBOX_CH_PROP & 0xFu);

	dmb_sy();
	while (MBOX_STATUS & MBOX_FULL) {
		__asm__ volatile("yield" ::: "memory");
	}
	MBOX_WRITE = bus;
	dmb_sy();

	for (;;) {
		while (MBOX_STATUS & MBOX_EMPTY) {
			__asm__ volatile("yield" ::: "memory");
		}
		dmb_sy();
		uint32_t rd = MBOX_READ;
		dmb_sy();
		if (rd == bus)
			return msg[1] == 0x80000000u;
	}
}
