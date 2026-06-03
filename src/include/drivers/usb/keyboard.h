#ifndef DRIVERS_USB_KEYBOARD_H
#define DRIVERS_USB_KEYBOARD_H

#include <drivers/usb/usb.h>

#define KEY_NONE       0
#define KEY_UP         0x100
#define KEY_DOWN       0x101
#define KEY_LEFT       0x102
#define KEY_RIGHT      0x103
#define KEY_HOME       0x104
#define KEY_END        0x105
#define KEY_PAGEUP     0x106
#define KEY_PAGEDOWN   0x107
#define KEY_DELETE      0x108

int usb_kbd_init(usb_device_t *dev);
int usb_kbd_poll(void);
int usb_kbd_available(void);

#endif
