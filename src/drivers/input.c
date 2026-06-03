// ==================================================================
// ARM-PRos - keyboard input (USB and UART when fallback)
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/input.h>
#include <drivers/uart.h>
#include <drivers/usb/usb.h>
#include <drivers/usb/keyboard.h>
#include <drivers/timer.h>
#include <log.h>

static int usb_available;

static int uart_trygetc(void)
{
	volatile unsigned int *fr = (volatile unsigned int *)(unsigned long)0x3F201018u;
	if (*fr & (1u << 4))
		return -1;
	volatile unsigned int *dr = (volatile unsigned int *)(unsigned long)0x3F201000u;
	return (int)(*dr & 0xFFu);
}

static int parse_uart_escape(void)
{
	delay_us(10000);
	int c2 = uart_trygetc();
	if (c2 < 0)
		return 0x1B;
	if (c2 != '[')
		return 0x1B;
	delay_us(10000);
	int c3 = uart_trygetc();
	if (c3 < 0)
		return 0x1B;
	switch (c3) {
	case 'A': return KEY_UP;
	case 'B': return KEY_DOWN;
	case 'C': return KEY_RIGHT;
	case 'D': return KEY_LEFT;
	case 'H': return KEY_HOME;
	case 'F': return KEY_END;
	case '5':
		delay_us(5000);
		uart_trygetc();
		return KEY_PAGEUP;
	case '6':
		delay_us(5000);
		uart_trygetc();
		return KEY_PAGEDOWN;
	case '3':
		delay_us(5000);
		uart_trygetc();
		return KEY_DELETE;
	}
	return 0x1B;
}

static void try_find_keyboard(void)
{
	int count = usb_get_device_count();
	for (int i = 0; i < count; i++) {
		usb_device_t *dev = usb_get_device(i);
		if (!dev || !dev->active)
			continue;
		if (dev->iface_class == USB_CLASS_HID &&
		    dev->iface_subclass == USB_HID_SUBCLASS_BOOT &&
		    dev->iface_protocol == USB_HID_PROTOCOL_KEYBOARD) {
			if (usb_kbd_init(dev) == 0) {
				usb_available = 1;
				return;
			}
		}
	}

	for (int i = 0; i < count; i++) {
		usb_device_t *dev = usb_get_device(i);
		if (!dev || !dev->active)
			continue;
		if (dev->iface_class == USB_CLASS_HID && dev->int_ep != 0) {
			if (usb_kbd_init(dev) == 0) {
				usb_available = 1;
				return;
			}
		}
	}
}

void input_init(void)
{
	usb_available = 0;

	if (usb_init() <= 0) {
		log_error("USB: DWC2 init failed");
		return;
	}

	int connected = 0;
	for (int attempt = 0; attempt < 10; attempt++) {
		delay_ms(200);
		if (usb_root_port_connected()) {
			connected = 1;
			break;
		}
	}

	if (!connected) {
		log_warn("USB: no device on root port");
		return;
	}

	if (!usb_root_port_reset()) {
		log_error("USB: root port reset failed");
		return;
	}

	delay_ms(50);

	usb_device_t *root_dev = usb_alloc_device();
	if (!root_dev) {
		log_error("USB: device slot alloc failed");
		return;
	}

	root_dev->speed = usb_root_port_speed();

	if (usb_enumerate_device(root_dev) < 0) {
		log_error("USB: device enumeration failed");
		return;
	}

	try_find_keyboard();

	if (!usb_available)
		log_error("USB: no HID keyboard interface found");
}

int input_poll(void)
{
	if (usb_available) {
		int key = usb_kbd_poll();
		if (key != KEY_NONE)
			return key;
	}

	int c = uart_trygetc();
	if (c < 0)
		return KEY_NONE;
	if (c == 0x1B)
		return parse_uart_escape();
	return c;
}

int input_getc(void)
{
	for (;;) {
		int key = input_poll();
		if (key != KEY_NONE)
			return key;
		delay_us(1000);
	}
}
