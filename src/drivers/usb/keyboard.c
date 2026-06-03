// ==================================================================
// ARM-PRos - USB HID boot-protocol keyboard driver for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/usb/keyboard.h>
#include <drivers/usb/usb.h>
#include <string.h>

static usb_device_t *kbd_dev;
static uint8_t kbd_ep;
static uint8_t kbd_mps;
static int kbd_ready;
static uint8_t prev_report[8];

static const char keymap_lower[128] = {
	0,0,0,0, 'a','b','c','d', 'e','f','g','h', 'i','j','k','l',
	'm','n','o','p', 'q','r','s','t', 'u','v','w','x', 'y','z','1','2',
	'3','4','5','6', '7','8','9','0', '\r',0x1B,'\b','\t', ' ','-','=','[',
	']','\\','#',';', '\'','`',',','.', '/',0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
};

static const char keymap_upper[128] = {
	0,0,0,0, 'A','B','C','D', 'E','F','G','H', 'I','J','K','L',
	'M','N','O','P', 'Q','R','S','T', 'U','V','W','X', 'Y','Z','!','@',
	'#','$','%','^', '&','*','(',')', '\r',0x1B,'\b','\t', ' ','_','+','{',
	'}','|','~',':', '"','~','<','>', '?',0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
};

#define HID_KEY_UP    0x52
#define HID_KEY_DOWN  0x51
#define HID_KEY_LEFT  0x50
#define HID_KEY_RIGHT 0x4F
#define HID_KEY_HOME  0x4A
#define HID_KEY_END   0x4D
#define HID_KEY_PGUP  0x4B
#define HID_KEY_PGDN  0x4E
#define HID_KEY_DEL   0x4C

static int translate_key(uint8_t keycode, uint8_t modifiers)
{
	switch (keycode) {
	case HID_KEY_UP:    return KEY_UP;
	case HID_KEY_DOWN:  return KEY_DOWN;
	case HID_KEY_LEFT:  return KEY_LEFT;
	case HID_KEY_RIGHT: return KEY_RIGHT;
	case HID_KEY_HOME:  return KEY_HOME;
	case HID_KEY_END:   return KEY_END;
	case HID_KEY_PGUP:  return KEY_PAGEUP;
	case HID_KEY_PGDN:  return KEY_PAGEDOWN;
	case HID_KEY_DEL:   return KEY_DELETE;
	}

	if (keycode >= 128)
		return KEY_NONE;

	int shift = (modifiers & 0x22u) ? 1 : 0;
	char c = shift ? keymap_upper[keycode] : keymap_lower[keycode];
	return c ? (int)(unsigned char)c : KEY_NONE;
}

int usb_kbd_init(usb_device_t *dev)
{
	kbd_dev = dev;
	kbd_ep = dev->int_ep;
	kbd_mps = dev->int_mps;
	if (kbd_mps == 0)
		kbd_mps = 8;
	dev->int_toggle = 0;
	kbd_ready = 1;
	memset(prev_report, 0, sizeof(prev_report));

	usb_control_msg(dev,
		USB_RT_HOST_TO_DEV | USB_RT_CLASS | USB_RT_INTERFACE,
		0x0A, 0, dev->iface_number, NULL, 0);

	usb_control_msg(dev,
		USB_RT_HOST_TO_DEV | USB_RT_CLASS | USB_RT_INTERFACE,
		0x0B, 0, dev->iface_number, NULL, 0);

	return 0;
}

int usb_kbd_poll(void)
{
	if (!kbd_ready || !kbd_dev)
		return KEY_NONE;

	uint8_t report[8];
	int rc = usb_int_transfer(kbd_dev, kbd_ep, report, 8, kbd_mps);
	if (rc < 0)
		return KEY_NONE;

	uint8_t modifiers = report[0];
	int key = KEY_NONE;

	for (int i = 2; i < 8; i++) {
		if (report[i] == 0)
			break;
		int found = 0;
		for (int j = 2; j < 8; j++) {
			if (prev_report[j] == report[i]) {
				found = 1;
				break;
			}
		}
		if (!found) {
			key = translate_key(report[i], modifiers);
			break;
		}
	}

	memcpy(prev_report, report, 8);
	return key;
}

int usb_kbd_available(void)
{
	return kbd_ready;
}
