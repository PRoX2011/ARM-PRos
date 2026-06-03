// ==================================================================
// ARM-PRos - DWC2 USB host controller (BCM2837) for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/usb/usb.h>
#include <drivers/mailbox.h>
#include <drivers/timer.h>
#include <string.h>
#include <stddef.h>

#define DWC2_BASE 0x3F980000ul

#define REG(off) (*(volatile uint32_t *)(uintptr_t)(DWC2_BASE + (off)))

#define GOTGCTL   0x000u
#define GOTGINT   0x004u
#define GAHBCFG   0x008u
#define GUSBCFG   0x00Cu
#define GRSTCTL   0x010u
#define GINTSTS   0x014u
#define GINTMSK   0x018u
#define GRXSTSP   0x020u
#define GRXFSIZ   0x024u
#define GNPTXFSIZ 0x028u
#define GSNPSID   0x040u
#define GHWCFG1   0x044u
#define GHWCFG2   0x048u
#define GHWCFG3   0x04Cu
#define GHWCFG4   0x050u
#define HPTXFSIZ  0x100u
#define HCFG      0x400u
#define HFIR      0x404u
#define HFNUM     0x408u
#define HPTXSTS   0x410u
#define HAINT     0x414u
#define HAINTMSK  0x418u
#define HPRT      0x440u

#define HCCHAR(n)    (0x500u + (n) * 0x20u)
#define HCSPLT(n)    (0x504u + (n) * 0x20u)
#define HCINT(n)     (0x508u + (n) * 0x20u)
#define HCINTMSK(n)  (0x50Cu + (n) * 0x20u)
#define HCTSIZ(n)    (0x510u + (n) * 0x20u)
#define HCDMA(n)     (0x514u + (n) * 0x20u)

#define PCGCCTL 0xE00u

#define GAHBCFG_GLBLINTRMSK   (1u << 0)
#define GAHBCFG_AXI_WAIT      (1u << 4)
#define GAHBCFG_DMAEN         (1u << 5)

#define GUSBCFG_PHYIF         (1u << 3)
#define GUSBCFG_ULPI_UTMI_SEL (1u << 4)
#define GUSBCFG_SRPCAP        (1u << 8)
#define GUSBCFG_HNPCAP        (1u << 9)
#define GUSBCFG_ULPI_FSLS     (1u << 17)
#define GUSBCFG_ULPI_CLK_SUS  (1u << 19)
#define GUSBCFG_ULPI_EXT_VBUS (1u << 20)
#define GUSBCFG_TERM_SEL      (1u << 22)
#define GUSBCFG_FORCEHOST     (1u << 29)
#define GUSBCFG_FORCEDEV      (1u << 30)

#define GRSTCTL_CSFTRST   (1u << 0)
#define GRSTCTL_AHBIDLE   (1u << 31)
#define GRSTCTL_TXFFLSH   (1u << 5)
#define GRSTCTL_RXFFLSH   (1u << 4)

#define GINTSTS_SOF       (1u << 3)
#define GINTSTS_RXFLVL    (1u << 4)
#define GINTSTS_PRTINT    (1u << 24)
#define GINTSTS_HCHINT    (1u << 25)

#define HPRT_CONNSTS      (1u << 0)
#define HPRT_CONNDET      (1u << 1)
#define HPRT_ENA          (1u << 2)
#define HPRT_ENCHNG       (1u << 3)
#define HPRT_OVRCURRCHG   (1u << 5)
#define HPRT_RST          (1u << 8)
#define HPRT_PWR          (1u << 12)
#define HPRT_SPD_MASK     (3u << 17)
#define HPRT_SPD_SHIFT    17

#define HPRT_WC_MASK (HPRT_CONNDET | HPRT_ENA | HPRT_ENCHNG | HPRT_OVRCURRCHG)

#define HCCHAR_MPS_MASK   0x7FFu
#define HCCHAR_EPNUM_SHIFT 11
#define HCCHAR_EPDIR_IN   (1u << 15)
#define HCCHAR_LSPDDEV    (1u << 17)
#define HCCHAR_EPTYPE_SHIFT 18
#define HCCHAR_MC_1       (1u << 20)
#define HCCHAR_DEVADDR_SHIFT 22
#define HCCHAR_ODDFRM     (1u << 29)
#define HCCHAR_CHDIS      (1u << 30)
#define HCCHAR_CHENA      (1u << 31)

#define HCTSIZ_PID_SHIFT  29

#define HCINT_XFERCOMPL   (1u << 0)
#define HCINT_CHHLTD      (1u << 1)
#define HCINT_AHBERR      (1u << 2)
#define HCINT_STALL       (1u << 3)
#define HCINT_NAK         (1u << 4)
#define HCINT_ACK         (1u << 5)
#define HCINT_NYET        (1u << 6)
#define HCINT_XACTERR     (1u << 7)
#define HCINT_BBLERR      (1u << 8)
#define HCINT_FRMOVRUN    (1u << 9)
#define HCINT_DATATGLERR  (1u << 10)

#define HCINT_ERROR_MASK (HCINT_AHBERR | HCINT_STALL | HCINT_XACTERR | HCINT_BBLERR | HCINT_FRMOVRUN | HCINT_DATATGLERR)

#define NUM_CHANNELS 8
#define TRANSFER_TIMEOUT_US 500000u
#define NAK_RETRY_LIMIT 500

static usb_device_t devices[USB_MAX_DEVICES];
static int device_count;
static uint8_t next_addr = 1;
static uint8_t root_speed = USB_SPEED_FULL;

static uint8_t __attribute__((aligned(4096))) dma_buf[512];

#define ARM_TO_BUS(p) ((uint32_t)(uintptr_t)(p) | 0xC0000000u)

static volatile uint32_t __attribute__((aligned(16))) usb_mbox[8];

static int usb_power_on(void)
{
	for (int attempt = 0; attempt < 5; attempt++) {
		usb_mbox[0] = 8 * 4;
		usb_mbox[1] = 0;
		usb_mbox[2] = 0x00028001u;
		usb_mbox[3] = 8;
		usb_mbox[4] = 0;
		usb_mbox[5] = 3;
		usb_mbox[6] = 3;
		usb_mbox[7] = 0;
		if (!mbox_call(usb_mbox)) {
			delay_ms(100);
			continue;
		}
		if (usb_mbox[6] & 0x2u)
			continue;
		if (usb_mbox[6] & 0x1u)
			return 1;
		delay_ms(100);
	}
	return 0;
}

static inline void dmb(void)
{
	__asm__ volatile("dmb sy" ::: "memory");
}

static inline void dsb(void)
{
	__asm__ volatile("dsb sy" ::: "memory");
}

static inline void clean_cache_line(void *ptr)
{
	__asm__ volatile("dc cvac, %0" :: "r"(ptr) : "memory");
}

static inline void invalidate_cache_line(void *ptr)
{
	__asm__ volatile("dc ivac, %0" :: "r"(ptr) : "memory");
}

static void dma_buf_flush(int len)
{
	for (int i = 0; i < len; i += 64)
		clean_cache_line((void *)((uintptr_t)dma_buf + i));
	dsb();
}

static void dma_buf_invalidate(int len)
{
	for (int i = 0; i < len; i += 64)
		invalidate_cache_line((void *)((uintptr_t)dma_buf + i));
	dsb();
}

static uint32_t hprt_read_clean(void)
{
	return REG(HPRT) & ~HPRT_WC_MASK;
}

static int core_reset(void)
{
	uint64_t t;

	t = timer_get_ticks() + 1000000u;
	while (!(REG(GRSTCTL) & GRSTCTL_AHBIDLE)) {
		if (timer_get_ticks() > t)
			return -1;
		delay_us(10);
	}

	REG(GRSTCTL) = GRSTCTL_CSFTRST;

	t = timer_get_ticks() + 1000000u;
	while (REG(GRSTCTL) & GRSTCTL_CSFTRST) {
		if (timer_get_ticks() > t)
			return -1;
		delay_us(10);
	}

	t = timer_get_ticks() + 1000000u;
	while (!(REG(GRSTCTL) & GRSTCTL_AHBIDLE)) {
		if (timer_get_ticks() > t)
			return -1;
		delay_us(10);
	}

	delay_ms(25);
	return 0;
}

static void flush_tx_fifo(uint32_t num)
{
	REG(GRSTCTL) = GRSTCTL_TXFFLSH | (num << 6);
	uint64_t t = timer_get_ticks() + 100000u;
	while ((REG(GRSTCTL) & GRSTCTL_TXFFLSH) && timer_get_ticks() < t)
		delay_us(1);
	delay_us(1);
}

static void flush_rx_fifo(void)
{
	REG(GRSTCTL) = GRSTCTL_RXFFLSH;
	uint64_t t = timer_get_ticks() + 100000u;
	while ((REG(GRSTCTL) & GRSTCTL_RXFFLSH) && timer_get_ticks() < t)
		delay_us(1);
	delay_us(1);
}

static void host_channel_init(void)
{
	for (int i = 0; i < NUM_CHANNELS; i++) {
		REG(HCINTMSK(i)) = 0;
		REG(HCINT(i)) = 0xFFFFFFFFu;
		uint32_t hcc = REG(HCCHAR(i));
		if (hcc & HCCHAR_CHENA) {
			hcc |= HCCHAR_CHDIS;
			hcc &= ~HCCHAR_CHENA;
			REG(HCCHAR(i)) = hcc;
			delay_us(100);
		}
		REG(HCINT(i)) = 0xFFFFFFFFu;
	}
}

#define HCSPLT_SPLTENA     (1u << 31)
#define HCSPLT_COMPSPLT    (1u << 14)
#define HCSPLT_XACTPOS_ALL (3u << 15)

static int wait_channel(int ch)
{
	uint64_t deadline = timer_get_ticks() + TRANSFER_TIMEOUT_US;
	while (timer_get_ticks() < deadline) {
		uint32_t intr = REG(HCINT(ch));

		if (intr & HCINT_CHHLTD) {
			REG(HCINT(ch)) = 0xFFFFFFFFu;
			if (intr & HCINT_XFERCOMPL)
				return 0;
			if (intr & HCINT_NYET)
				return -6;
			if (intr & HCINT_NAK)
				return -3;
			if (intr & HCINT_STALL)
				return -2;
			if (intr & HCINT_ERROR_MASK)
				return -4;
			if (intr & HCINT_ACK)
				return 0;
			return -5;
		}

		if (intr & HCINT_XFERCOMPL) {
			REG(HCINT(ch)) = 0xFFFFFFFFu;
			return 0;
		}

		delay_us(5);
	}

	REG(HCCHAR(ch)) = REG(HCCHAR(ch)) | HCCHAR_CHDIS | HCCHAR_CHENA;
	delay_ms(1);
	REG(HCINT(ch)) = 0xFFFFFFFFu;
	return -1;
}

static void setup_channel(int ch, uint32_t hcchar, uint32_t hctsiz,
                           uint32_t hcsplt, void *buf)
{
	REG(HCINT(ch)) = 0xFFFFFFFFu;
	REG(HCINTMSK(ch)) = HCINT_XFERCOMPL | HCINT_CHHLTD | HCINT_ERROR_MASK | HCINT_NAK | HCINT_ACK | HCINT_NYET;
	REG(HCSPLT(ch)) = hcsplt;
	REG(HCTSIZ(ch)) = hctsiz;
	REG(HCDMA(ch)) = ARM_TO_BUS(buf);
	dmb();
	dsb();
	REG(HCCHAR(ch)) = hcchar | HCCHAR_CHENA;
	dsb();
}

static int do_transfer(int ch, uint8_t devaddr, uint8_t ep, uint8_t dir,
                       uint8_t type, uint8_t pid, void *buf, int len,
                       int mps, int lowspeed,
                       uint8_t hub_addr, uint8_t hub_port)
{
	if (mps == 0)
		mps = 8;

	int pktcnt = (len + mps - 1) / mps;
	if (pktcnt == 0)
		pktcnt = 1;

	uint32_t hcchar = (uint32_t)(mps & HCCHAR_MPS_MASK)
	                | ((uint32_t)(ep & 0xFu) << HCCHAR_EPNUM_SHIFT)
	                | ((uint32_t)(type & 0x3u) << HCCHAR_EPTYPE_SHIFT)
	                | ((uint32_t)(devaddr & 0x7Fu) << HCCHAR_DEVADDR_SHIFT)
	                | HCCHAR_MC_1;
	if (dir == USB_DIR_IN)
		hcchar |= HCCHAR_EPDIR_IN;
	if (lowspeed)
		hcchar |= HCCHAR_LSPDDEV;

	uint32_t hctsiz = ((uint32_t)len & 0x7FFFFu)
	                | ((uint32_t)pktcnt << 19)
	                | ((uint32_t)(pid & 0x3u) << HCTSIZ_PID_SHIFT);

	if (hub_addr == 0 || root_speed != USB_SPEED_HIGH) {
		setup_channel(ch, hcchar, hctsiz, 0, buf);
		return wait_channel(ch);
	}

	uint32_t hcsplt = HCSPLT_SPLTENA | HCSPLT_XACTPOS_ALL
	                | ((uint32_t)(hub_addr & 0x7Fu) << 7)
	                | ((uint32_t)(hub_port & 0x7Fu));

	for (int ssplit = 0; ssplit < 3; ssplit++) {
		setup_channel(ch, hcchar, hctsiz, hcsplt, buf);
		int rc = wait_channel(ch);
		if (rc == -3) {
			delay_us(125);
			continue;
		}
		if (rc < 0 && rc != 0)
			return rc;

		for (int csplit = 0; csplit < 10; csplit++) {
			delay_us(125);
			setup_channel(ch, hcchar, hctsiz,
			              hcsplt | HCSPLT_COMPSPLT, buf);
			rc = wait_channel(ch);
			if (rc == 0)
				return 0;
			if (rc == -6) {
				delay_us(125);
				continue;
			}
			if (rc == -3)
				break;
			return rc;
		}
	}
	return -1;
}

int usb_init(void)
{
	memset(devices, 0, sizeof(devices));
	device_count = 0;
	next_addr = 1;

	(void)usb_power_on();
	delay_ms(250);

	REG(PCGCCTL) = 0;
	delay_ms(10);

	uint32_t snpsid = REG(GSNPSID);
	if (snpsid == 0u)
		return 0;

	uint32_t snpsid_masked = snpsid & 0xFFFFF000u;
	if (snpsid_masked != 0x4F542000u && snpsid_masked != 0x4F543000u)
		return 0;

	uint32_t gusbcfg = REG(GUSBCFG);
	gusbcfg &= ~GUSBCFG_ULPI_EXT_VBUS;
	gusbcfg &= ~GUSBCFG_TERM_SEL;
	REG(GUSBCFG) = gusbcfg;

	if (core_reset() < 0)
		return 0;

	REG(PCGCCTL) = 0;
	delay_ms(10);

	gusbcfg = REG(GUSBCFG);
	gusbcfg &= ~GUSBCFG_ULPI_UTMI_SEL;
	gusbcfg &= ~GUSBCFG_PHYIF;
	gusbcfg &= ~GUSBCFG_ULPI_FSLS;
	gusbcfg &= ~GUSBCFG_ULPI_CLK_SUS;
	gusbcfg &= ~(GUSBCFG_SRPCAP | GUSBCFG_HNPCAP);
	gusbcfg &= ~(0xFu << 10);
	gusbcfg |= (9u << 10);
	REG(GUSBCFG) = gusbcfg;

	REG(GAHBCFG) = GAHBCFG_GLBLINTRMSK | GAHBCFG_DMAEN | GAHBCFG_AXI_WAIT;

	gusbcfg = REG(GUSBCFG);
	gusbcfg |= GUSBCFG_FORCEHOST;
	gusbcfg &= ~GUSBCFG_FORCEDEV;
	REG(GUSBCFG) = gusbcfg;
	delay_ms(200);

	REG(HCFG) = (REG(HCFG) & ~3u) | 1u;

	REG(GRXFSIZ) = 1024u;
	REG(GNPTXFSIZ) = (1024u << 16) | 1024u;
	REG(HPTXFSIZ) = (1024u << 16) | 2048u;

	flush_tx_fifo(0x10);
	flush_rx_fifo();

	host_channel_init();

	REG(HAINTMSK) = (1u << NUM_CHANNELS) - 1u;
	REG(GINTMSK) = GINTSTS_PRTINT | GINTSTS_HCHINT | GINTSTS_SOF | GINTSTS_RXFLVL;
	REG(GINTSTS) = 0xFFFFFFFFu;

	uint32_t hp = hprt_read_clean();
	if (hp & HPRT_OVRCURRCHG)
		REG(HPRT) = (hp & ~HPRT_WC_MASK) | HPRT_OVRCURRCHG;

	hp = hprt_read_clean();
	hp |= HPRT_PWR;
	REG(HPRT) = hp;
	delay_ms(100);

	if (!(REG(HPRT) & HPRT_PWR)) {
		hp = hprt_read_clean();
		hp |= HPRT_PWR;
		REG(HPRT) = hp;
		delay_ms(100);
	}

	delay_ms(200);
	return 1;
}

int usb_root_port_connected(void)
{
	return (REG(HPRT) & HPRT_CONNSTS) ? 1 : 0;
}

int usb_root_port_reset(void)
{
	uint32_t hp = REG(HPRT);
	if (hp & HPRT_CONNDET)
		REG(HPRT) = (hp & ~HPRT_WC_MASK) | HPRT_CONNDET;
	if (hp & HPRT_OVRCURRCHG)
		REG(HPRT) = (hp & ~HPRT_WC_MASK) | HPRT_OVRCURRCHG;

	hp = hprt_read_clean();
	hp |= HPRT_PWR | HPRT_RST;
	REG(HPRT) = hp;
	delay_ms(60);

	hp = hprt_read_clean();
	hp &= ~HPRT_RST;
	REG(HPRT) = hp;

	for (int i = 0; i < 50; i++) {
		delay_ms(10);
		uint32_t val = REG(HPRT);
		if (val & HPRT_ENCHNG)
			REG(HPRT) = (val & ~HPRT_WC_MASK) | HPRT_ENCHNG;
		if (val & HPRT_OVRCURRCHG)
			REG(HPRT) = (val & ~HPRT_WC_MASK) | HPRT_OVRCURRCHG;
		if (val & HPRT_ENA)
			return 1;
	}
	return 0;
}

uint8_t usb_root_port_speed(void)
{
	root_speed = (uint8_t)((REG(HPRT) & HPRT_SPD_MASK) >> HPRT_SPD_SHIFT);
	return root_speed;
}

int usb_control_msg(usb_device_t *dev, uint8_t rt, uint8_t req,
                    uint16_t val, uint16_t idx, void *data, uint16_t len)
{
	int ch = 0;
	int ls = (dev->speed == USB_SPEED_LOW) ? 1 : 0;
	uint8_t ha = dev->parent_addr;
	uint8_t hp = dev->parent_port;

	usb_setup_t __attribute__((aligned(4))) setup;
	setup.bmRequestType = rt;
	setup.bRequest = req;
	setup.wValue = val;
	setup.wIndex = idx;
	setup.wLength = len;

	memcpy(dma_buf, &setup, 8);
	dma_buf_flush(8);
	dmb();
	dsb();

	int rc = do_transfer(ch, dev->addr, 0, USB_DIR_OUT, USB_EP_CONTROL,
	                     USB_PID_SETUP, dma_buf, 8, dev->mps0, ls, ha, hp);
	if (rc < 0)
		return rc;

	if (len > 0 && data) {
		uint8_t dir = (rt & USB_RT_DEV_TO_HOST) ? USB_DIR_IN : USB_DIR_OUT;

		if (dir == USB_DIR_OUT) {
			memcpy(dma_buf, data, len);
			dma_buf_flush(len);
		} else {
			memset(dma_buf, 0, (size_t)len);
			dma_buf_flush(len);
		}
		dmb();
		dsb();

		rc = -3;
		for (int retry = 0; retry < NAK_RETRY_LIMIT; retry++) {
			rc = do_transfer(ch, dev->addr, 0, dir, USB_EP_CONTROL,
			                 USB_PID_DATA1, dma_buf, len, dev->mps0, ls, ha, hp);
			if (rc == 0)
				break;
			if (rc != -3)
				return rc;
			delay_us(250);
		}
		if (rc < 0)
			return rc;

		if (dir == USB_DIR_IN) {
			dma_buf_invalidate(len);
			dmb();
			memcpy(data, dma_buf, len);
		}
	}

	{
		uint8_t status_dir = (rt & USB_RT_DEV_TO_HOST) ? USB_DIR_OUT : USB_DIR_IN;
		rc = -3;
		for (int retry = 0; retry < NAK_RETRY_LIMIT; retry++) {
			rc = do_transfer(ch, dev->addr, 0, status_dir, USB_EP_CONTROL,
			                 USB_PID_DATA1, dma_buf, 0, dev->mps0, ls, ha, hp);
			if (rc == 0)
				break;
			if (rc != -3)
				return rc;
			delay_us(250);
		}
	}

	return rc;
}

int usb_int_transfer(usb_device_t *dev, uint8_t ep, void *data, int len, int mps)
{
	int ch = 1;
	int ls = (dev->speed == USB_SPEED_LOW) ? 1 : 0;
	uint8_t ha = dev->parent_addr;
	uint8_t hp = dev->parent_port;

	memset(dma_buf, 0, (size_t)len);
	dma_buf_flush(len);
	dmb();
	dsb();

	uint8_t pid = dev->int_toggle ? USB_PID_DATA1 : USB_PID_DATA0;

	int rc = do_transfer(ch, dev->addr, ep & 0x7Fu, USB_DIR_IN,
	                     USB_EP_INTERRUPT, pid, dma_buf, len, mps, ls,
	                     ha, hp);
	if (rc == 0) {
		dev->int_toggle ^= 1u;
		dma_buf_invalidate(len);
		dmb();
		memcpy(data, dma_buf, (size_t)len);
	}
	return rc;
}

usb_device_t *usb_alloc_device(void)
{
	if (device_count >= USB_MAX_DEVICES)
		return NULL;
	usb_device_t *dev = &devices[device_count];
	memset(dev, 0, sizeof(*dev));
	dev->active = 1;
	dev->mps0 = 8;
	device_count++;
	return dev;
}

usb_device_t *usb_get_device(int index)
{
	if (index < 0 || index >= device_count)
		return NULL;
	return &devices[index];
}

int usb_get_device_count(void)
{
	return device_count;
}

int usb_enumerate_device(usb_device_t *dev)
{
	usb_dev_desc_t ddesc;
	int rc;

	rc = usb_control_msg(dev, USB_RT_DEV_TO_HOST | USB_RT_STANDARD | USB_RT_DEVICE,
	                     USB_REQ_GET_DESCRIPTOR, (USB_DESC_DEVICE << 8), 0,
	                     &ddesc, 8);
	if (rc < 0)
		return rc;

	dev->mps0 = ddesc.bMaxPacketSize0;
	if (dev->mps0 == 0)
		dev->mps0 = 8;

	uint8_t new_addr = next_addr++;
	rc = usb_control_msg(dev, USB_RT_HOST_TO_DEV | USB_RT_STANDARD | USB_RT_DEVICE,
	                     USB_REQ_SET_ADDRESS, new_addr, 0, NULL, 0);
	if (rc < 0)
		return rc;

	dev->addr = new_addr;
	delay_ms(20);

	rc = usb_control_msg(dev, USB_RT_DEV_TO_HOST | USB_RT_STANDARD | USB_RT_DEVICE,
	                     USB_REQ_GET_DESCRIPTOR, (USB_DESC_DEVICE << 8), 0,
	                     &ddesc, sizeof(ddesc));
	if (rc < 0)
		return rc;

	dev->vid = ddesc.idVendor;
	dev->pid = ddesc.idProduct;
	dev->dev_class = ddesc.bDeviceClass;
	dev->dev_subclass = ddesc.bDeviceSubClass;
	dev->dev_protocol = ddesc.bDeviceProtocol;

	uint8_t __attribute__((aligned(4))) config_buf[128];
	memset(config_buf, 0, sizeof(config_buf));
	rc = usb_control_msg(dev, USB_RT_DEV_TO_HOST | USB_RT_STANDARD | USB_RT_DEVICE,
	                     USB_REQ_GET_DESCRIPTOR, (USB_DESC_CONFIG << 8), 0,
	                     config_buf, sizeof(config_buf));
	if (rc < 0)
		return rc;

	usb_config_desc_t *cfg = (usb_config_desc_t *)config_buf;

	int offset = cfg->bLength;
	int total = cfg->wTotalLength;
	if (total > (int)sizeof(config_buf))
		total = (int)sizeof(config_buf);

	uint8_t cur_class = 0, cur_sub = 0, cur_proto = 0, cur_num = 0;
	int cur_is_kbd = 0;
	int kbd_found = 0;

	int fb_valid = 0;
	uint8_t fb_ep = 0, fb_mps = 0, fb_intv = 0, fb_num = 0;
	uint8_t fb_class = 0, fb_sub = 0, fb_proto = 0;

	for (int iter = 0; iter < 32 && offset + 2 <= total; iter++) {
		uint8_t dlen = config_buf[offset];
		uint8_t dtype = config_buf[offset + 1];
		if (dlen < 2 || offset + dlen > total)
			break;

		if (dtype == USB_DESC_INTERFACE && dlen >= 9) {
			usb_iface_desc_t *iface = (usb_iface_desc_t *)&config_buf[offset];
			cur_num = iface->bInterfaceNumber;
			cur_class = iface->bInterfaceClass;
			cur_sub = iface->bInterfaceSubClass;
			cur_proto = iface->bInterfaceProtocol;
			cur_is_kbd = (cur_class == USB_CLASS_HID &&
			              cur_sub == USB_HID_SUBCLASS_BOOT &&
			              cur_proto == USB_HID_PROTOCOL_KEYBOARD);
		}

		if (dtype == USB_DESC_ENDPOINT && dlen >= 7) {
			usb_ep_desc_t *ep = (usb_ep_desc_t *)&config_buf[offset];
			int int_in = ((ep->bmAttributes & 3u) == USB_EP_INTERRUPT) &&
			             (ep->bEndpointAddress & 0x80u);
			if (int_in && cur_is_kbd && !kbd_found) {
				dev->int_ep = ep->bEndpointAddress & 0x7Fu;
				dev->int_mps = (uint8_t)(ep->wMaxPacketSize & 0xFFu);
				dev->int_interval = ep->bInterval;
				dev->iface_class = cur_class;
				dev->iface_subclass = cur_sub;
				dev->iface_protocol = cur_proto;
				dev->iface_number = cur_num;
				kbd_found = 1;
			} else if (int_in && !fb_valid && cur_class == USB_CLASS_HID) {
				fb_ep = ep->bEndpointAddress & 0x7Fu;
				fb_mps = (uint8_t)(ep->wMaxPacketSize & 0xFFu);
				fb_intv = ep->bInterval;
				fb_num = cur_num;
				fb_class = cur_class;
				fb_sub = cur_sub;
				fb_proto = cur_proto;
				fb_valid = 1;
			}
		}

		offset += dlen;
	}

	if (!kbd_found && fb_valid) {
		dev->int_ep = fb_ep;
		dev->int_mps = fb_mps;
		dev->int_interval = fb_intv;
		dev->iface_class = fb_class;
		dev->iface_subclass = fb_sub;
		dev->iface_protocol = fb_proto;
		dev->iface_number = fb_num;
	}

	rc = usb_control_msg(dev, USB_RT_HOST_TO_DEV | USB_RT_STANDARD | USB_RT_DEVICE,
	                     USB_REQ_SET_CONFIG, cfg->bConfigurationValue, 0, NULL, 0);
	if (rc < 0)
		return rc;

	delay_ms(10);
	return 0;
}
