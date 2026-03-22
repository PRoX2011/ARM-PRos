#include <drivers/usb.h>
#include <drivers/uart.h>
#include <drivers/timer.h>
#include <stdlib.h>

/* DWC2 USB Controller Base Address (Raspberry Pi 3) */
#define USB_BASE                0x3F980000u

/* Global Registers */
#define USB_GOTGCTL             (*(volatile unsigned int *)(USB_BASE + 0x00u))
#define USB_GOTGINT             (*(volatile unsigned int *)(USB_BASE + 0x04u))
#define USB_GAHBCFG             (*(volatile unsigned int *)(USB_BASE + 0x08u))
#define USB_GUSBCFG             (*(volatile unsigned int *)(USB_BASE + 0x0Cu))
#define USB_GRSTCTL             (*(volatile unsigned int *)(USB_BASE + 0x10u))
#define USB_GINTSTS             (*(volatile unsigned int *)(USB_BASE + 0x14u))
#define USB_GINTMSK             (*(volatile unsigned int *)(USB_BASE + 0x18u))
#define USB_GRXSTSR             (*(volatile unsigned int *)(USB_BASE + 0x1Cu))
#define USB_GRXFSIZ             (*(volatile unsigned int *)(USB_BASE + 0x24u))
#define USB_GNPTXFSIZ           (*(volatile unsigned int *)(USB_BASE + 0x28u))
#define USB_GNPTXSTS            (*(volatile unsigned int *)(USB_BASE + 0x2Cu))
#define USB_GCCFG               (*(volatile unsigned int *)(USB_BASE + 0x100u))
#define USB_GUID                (*(volatile unsigned int *)(USB_BASE + 0x140u))
#define USB_GSNPSID             (*(volatile unsigned int *)(USB_BASE + 0x140u))

/* Host Global Registers */
#define USB_HCFG                (*(volatile unsigned int *)(USB_BASE + 0x400u))
#define USB_HFIR                (*(volatile unsigned int *)(USB_BASE + 0x404u))
#define USB_HFNUM               (*(volatile unsigned int *)(USB_BASE + 0x408u))
#define USB_HPTXSTS             (*(volatile unsigned int *)(USB_BASE + 0x410u))
#define USB_HAINT               (*(volatile unsigned int *)(USB_BASE + 0x414u))
#define USB_HAINTMSK            (*(volatile unsigned int *)(USB_BASE + 0x418u))
#define USB_HFLBADDR            (*(volatile unsigned int *)(USB_BASE + 0x41Cu))

/* Host Port Control and Status Register */
#define USB_HPRT0               (*(volatile unsigned int *)(USB_BASE + 0x440u))

/* Host Channel Registers (Channel 0) */
#define USB_HCCHAR0             (*(volatile unsigned int *)(USB_BASE + 0x500u))
#define USB_HCSPLT0             (*(volatile unsigned int *)(USB_BASE + 0x504u))
#define USB_HCINT0              (*(volatile unsigned int *)(USB_BASE + 0x508u))
#define USB_HCINTMSK0           (*(volatile unsigned int *)(USB_BASE + 0x50Cu))
#define USB_HCTSIZ0             (*(volatile unsigned int *)(USB_BASE + 0x510u))
#define USB_HCDMA0              (*(volatile unsigned int *)(USB_BASE + 0x514u))

/* GRSTCTL Bits */
#define GRSTCTL_CSRST           (1u << 0)   /* Core Soft Reset */
#define GRSTCTL_HSRST           (1u << 1)   /* HCLK Soft Reset */
#define GRSTCTL_FCRST           (1u << 2)   /* Fifo Flush */
#define GRSTCTL_AHBIDLE         (1u << 31)  /* AHB Master is Idle */

/* GAHBCFG Bits */
#define GAHBCFG_GLBLINTRMSK     (1u << 0)   /* Global Interrupt Mask */
#define GAHBCFG_HBSTLEN_SHIFT   1
#define GAHBCFG_HBSTLEN_MASK    (0xFu << GAHBCFG_HBSTLEN_SHIFT)
#define GAHBCFG_DMAEN           (1u << 5)   /* DMA Enable */

/* GUSBCFG Bits */
#define GUSBCFG_TOUTCAL_SHIFT   0
#define GUSBCFG_TOUTCAL_MASK    (0x7u << GUSBCFG_TOUTCAL_SHIFT)
#define GUSBCFG_PHYIF           (1u << 3)   /* PHY Interface (0=8-bit, 1=16-bit) */
#define GUSBCFG_ULPI_UTMI_SEL   (1u << 4)   /* ULPI or UTMI+ Selection */
#define GUSBCFG_FSINTF          (1u << 5)   /* Full-Speed Serial Interface Select */
#define GUSBCFG_PHYSEL          (1u << 6)   /* USB 1.1 Full-Speed Serial Transceiver Select */
#define GUSBCFG_SRPCAP          (1u << 12)  /* SRP-Capable */
#define GUSBCFG_HNPCAP          (1u << 13)  /* HNP-Capable */
#define GUSBCFG_FHMOD           (1u << 29)  /* Force Host Mode */
#define GUSBCFG_FDMOD           (1u << 30)  /* Force Client Mode */

/* GCCFG Bits */
#define GCCFG_PWRDOWN           (1u << 16)  /* Power Down */
#define GCCFG_VBUSVLD           (1u << 21)  /* VBUS Valid */
#define GCCFG_SVALID            (1u << 22)  /* Session Valid */
#define GCCFG_BVALID            (1u << 23)  /* B-Session Valid */
#define GCCFG_AVALID            (1u << 24)  /* A-Session Valid */
#define GCCFG_OTGVER            (1u << 20)  /* OTG Version */

/* HCFG Bits */
#define HCFG_FSLSPCLKSEL_SHIFT  0
#define HCFG_FSLSPCLKSEL_MASK   (0x3u << HCFG_FSLSPCLKSEL_SHIFT)
#define HCFG_FSLSPCLKSEL_30_60  (0u << HCFG_FSLSPCLKSEL_SHIFT)
#define HCFG_FSLSPCLKSEL_60     (1u << HCFG_FSLSPCLKSEL_SHIFT)
#define HCFG_FSLSSUPP           (1u << 2)   /* FS/LS-only Support */

/* HPRT0 Bits */
#define HPRT0_PRTCONNSTS        (1u << 0)   /* Port Connected Status */
#define HPRT0_PRTCONNDET        (1u << 1)   /* Port Connect Detected */
#define HPRT0_PRTENA            (1u << 2)   /* Port Enable */
#define HPRT0_PRTENCHG          (1u << 3)   /* Port Enable/Disable Change */
#define HPRT0_PRTOVRCURRACT     (1u << 4)   /* Port Overcurrent Active */
#define HPRT0_PRTOVRCURRCHG     (1u << 5)   /* Port Overcurrent Change */
#define HPRT0_PRTRES            (1u << 6)   /* Port Resume */
#define HPRT0_PRTSUSP           (1u << 7)   /* Port Suspend */
#define HPRT0_PRTRST            (1u << 8)   /* Port Reset */
#define HPRT0_PRTSPD_SHIFT      17
#define HPRT0_PRTSPD_MASK       (0x3u << HPRT0_PRTSPD_SHIFT)
#define HPRT0_PRTSPD_HS         (0u << HPRT0_PRTSPD_SHIFT)
#define HPRT0_PRTSPD_FS         (1u << HPRT0_PRTSPD_SHIFT)
#define HPRT0_PRTSPD_LS         (2u << HPRT0_PRTSPD_SHIFT)

/* USB Device States */
typedef enum {
    USB_STATE_DISCONNECTED,
    USB_STATE_ATTACHED,
    USB_STATE_POWERED,
    USB_STATE_DEFAULT,
    USB_STATE_ADDRESSED,
    USB_STATE_CONFIGURED
} usb_device_state_t;

/* USB Device Info */
typedef struct {
    uint8_t address;
    uint8_t config;
    uint8_t interface;
    uint8_t endpoint_in;
    uint8_t endpoint_out;
    usb_device_state_t state;
} usb_device_t;

static volatile int keyboard_ready = 0;

/* HID scancode to ASCII lookup table */
static const char scancode_to_ascii[128] = {
    0x00, 0x00, 0x00, 0x00, 'a', 'b', 'c', 'd',
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
    'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '0',
    '\r', '\x1B', '\b', '\t', ' ', '-', '=', '[',
    ']', '\\', 0x00, ';', '\'', '`', ',', '.',
    '/', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const char scancode_to_ascii_shift[128] = {
    0x00, 0x00, 0x00, 0x00, 'A', 'B', 'C', 'D',
    'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',
    '#', '$', '%', '^', '&', '*', '(', ')',
    '\r', '\x1B', '\b', '\t', ' ', '_', '+', '{',
    '}', '|', 0x00, ':', '"', '~', '<', '>',
    '?', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define HID_MOD_LCTRL   0x01
#define HID_MOD_LSHIFT  0x02
#define HID_MOD_LALT    0x04
#define HID_MOD_LMETA   0x08
#define HID_MOD_RCTRL   0x10
#define HID_MOD_RSHIFT  0x20
#define HID_MOD_RALT    0x40
#define HID_MOD_RMETA   0x80

static void usb_core_reset(void)
{
    uart_puts("[DWC2] Core reset...\n\r");
    
    /* Wait for AHB to become idle */
    unsigned timeout = 1000000;
    while ((USB_GRSTCTL & GRSTCTL_AHBIDLE) == 0 && timeout-- > 0) {
        __asm__("nop");
    }
    
    if (timeout == 0) {
        uart_puts("[DWC2] ERROR: AHB not idle!\n\r");
        return;
    }
    
    /* Perform core soft reset */
    USB_GRSTCTL |= GRSTCTL_CSRST;
    
    timeout = 1000000;
    while ((USB_GRSTCTL & GRSTCTL_CSRST) != 0 && timeout-- > 0) {
        __asm__("nop");
    }
    
    if (timeout == 0) {
        uart_puts("[DWC2] ERROR: Core reset timeout!\n\r");
        return;
    }
    
    uart_puts("[DWC2] Core reset complete\n\r");
    timer_delay_ms(100);
}

static void usb_phy_init(void)
{
    uart_puts("[DWC2] Initializing PHY...\n\r");
    
    /* Disable power down */
    unsigned gccfg = USB_GCCFG;
    gccfg &= ~GCCFG_PWRDOWN;
    gccfg |= GCCFG_VBUSVLD | GCCFG_BVALID;
    USB_GCCFG = gccfg;
    
    timer_delay_ms(50);
    uart_puts("[DWC2] PHY initialized\n\r");
}

static void usb_host_init(void)
{
    uart_puts("[DWC2] Initializing host mode...\n\r");
    
    /* Set host mode */
    unsigned gusbcfg = USB_GUSBCFG;
    gusbcfg |= GUSBCFG_FHMOD;
    gusbcfg &= ~GUSBCFG_FDMOD;
    USB_GUSBCFG = gusbcfg;
    
    timer_delay_ms(50);
    
    /* Configure HCFG */
    unsigned hcfg = USB_HCFG;
    hcfg &= ~HCFG_FSLSPCLKSEL_MASK;
    hcfg |= HCFG_FSLSPCLKSEL_30_60;
    USB_HCFG = hcfg;
    
    /* Enable host interrupts */
    USB_GINTSTS = 0xFFFFFFFFu;  /* Clear all interrupts */
    USB_GINTMSK = (1u << 1) |   /* RESET_DET */
                  (1u << 2) |   /* SOF */
                  (1u << 5) |   /* NPTXFE */
                  (1u << 6) |   /* RXFLVL */
                  (1u << 24);   /* HCINT */
    
    /* Enable global interrupt */
    USB_GAHBCFG |= GAHBCFG_GLBLINTRMSK;
    
    uart_puts("[DWC2] Host mode initialized\n\r");
}

void usb_init(void)
{
    uart_puts("[DWC2] Starting USB DWC2 initialization...\n\r");
    
    uart_puts("[DWC2] USB base: ");
    uart_puthex((uint64_t)USB_BASE);
    uart_puts("\n\r");
    
    /* Try to read device ID from different offsets */
    unsigned id1 = USB_GSNPSID;
    uart_puts("[DWC2] GSNPSID at +0x140: ");
    uart_puthex((uint64_t)id1);
    uart_puts("\n\r");
    
    /* Alternative offset */
    unsigned *alt_id = (unsigned *)(USB_BASE + 0x50u);
    uart_puts("[DWC2] OTG Version at +0x50: ");
    uart_puthex((uint64_t)*alt_id);
    uart_puts("\n\r");
    
    /* Check HCFG */
    unsigned hcfg_val = USB_HCFG;
    uart_puts("[DWC2] HCFG at +0x400: ");
    uart_puthex((uint64_t)hcfg_val);
    uart_puts("\n\r");
    
    if (id1 == 0xFFFFFFFFu || id1 == 0x00000000u) {
        uart_puts("[DWC2] WARNING: Device ID is invalid - USB may not be emulated by QEMU\n\r");
        uart_puts("[DWC2] Continuing anyway...\n\r");
    }
    
    usb_core_reset();
    usb_phy_init();
    usb_host_init();
    
    /* Try port detection */
    uart_puts("[DWC2] Checking port status (HPRT0)...\n\r");
    unsigned hprt = USB_HPRT0;
    uart_puts("[DWC2] HPRT0: ");
    uart_puthex((uint64_t)hprt);
    uart_puts("\n\r");
    
    if (hprt & HPRT0_PRTCONNSTS) {
        uart_puts("[DWC2] Device connected on port!\n\r");
        keyboard_ready = 1;
    } else {
        uart_puts("[DWC2] No device detected on port\n\r");
    }
    
    uart_puts("[DWC2] USB initialization complete\n\r");
}

int usb_keyboard_connected(void)
{
    return keyboard_ready;
}

char usb_scancode_to_ascii(uint8_t scancode, uint8_t modifier)
{
    const char *table = (modifier & (HID_MOD_LSHIFT | HID_MOD_RSHIFT)) 
                        ? scancode_to_ascii_shift 
                        : scancode_to_ascii;
    
    if (scancode < 128)
        return table[scancode];
    
    return 0;
}

/* This is a stub - would use USB HID data */
unsigned char usb_keyboard_getc(void)
{
    /* For QEMU with proper USB, this would read HID reports */
    return 0;
}
