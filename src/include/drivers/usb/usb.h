#ifndef DRIVERS_USB_USB_H
#define DRIVERS_USB_USB_H

#include <stdint.h>

#define USB_SPEED_HIGH 0
#define USB_SPEED_FULL 1
#define USB_SPEED_LOW  2

#define USB_DIR_OUT 0
#define USB_DIR_IN  1

#define USB_EP_CONTROL   0
#define USB_EP_ISOC      1
#define USB_EP_BULK      2
#define USB_EP_INTERRUPT 3

#define USB_PID_DATA0 0
#define USB_PID_DATA2 1
#define USB_PID_DATA1 2
#define USB_PID_SETUP 3

#define USB_REQ_GET_STATUS     0x00
#define USB_REQ_CLEAR_FEATURE  0x01
#define USB_REQ_SET_FEATURE    0x03
#define USB_REQ_SET_ADDRESS    0x05
#define USB_REQ_GET_DESCRIPTOR 0x06
#define USB_REQ_SET_CONFIG     0x09
#define USB_REQ_SET_INTERFACE  0x0B

#define USB_DESC_DEVICE        0x01
#define USB_DESC_CONFIG        0x02
#define USB_DESC_STRING        0x03
#define USB_DESC_INTERFACE     0x04
#define USB_DESC_ENDPOINT      0x05

#define USB_RT_HOST_TO_DEV     0x00
#define USB_RT_DEV_TO_HOST     0x80
#define USB_RT_STANDARD        0x00
#define USB_RT_CLASS           0x20
#define USB_RT_VENDOR          0x40
#define USB_RT_DEVICE          0x00
#define USB_RT_INTERFACE       0x01
#define USB_RT_ENDPOINT        0x02
#define USB_RT_OTHER           0x03

#define USB_CLASS_HID 0x03

#define USB_HID_SUBCLASS_BOOT     1
#define USB_HID_PROTOCOL_KEYBOARD 1

#define USB_MAX_DEVICES 8

typedef struct {
	uint8_t  bmRequestType;
	uint8_t  bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__((packed)) usb_setup_t;

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t  bDeviceSubClass;
	uint8_t  bDeviceProtocol;
	uint8_t  bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t  iProduct;
	uint8_t  iSerialNumber;
	uint8_t  bNumConfigurations;
} __attribute__((packed)) usb_dev_desc_t;

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t wTotalLength;
	uint8_t  bNumInterfaces;
	uint8_t  bConfigurationValue;
	uint8_t  iConfiguration;
	uint8_t  bmAttributes;
	uint8_t  bMaxPower;
} __attribute__((packed)) usb_config_desc_t;

typedef struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__((packed)) usb_iface_desc_t;

typedef struct {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bEndpointAddress;
	uint8_t  bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t  bInterval;
} __attribute__((packed)) usb_ep_desc_t;

typedef struct {
	uint8_t  active;
	uint8_t  addr;
	uint8_t  speed;
	uint8_t  mps0;
	uint16_t vid;
	uint16_t pid;
	uint8_t  dev_class;
	uint8_t  dev_subclass;
	uint8_t  dev_protocol;
	uint8_t  iface_class;
	uint8_t  iface_subclass;
	uint8_t  iface_protocol;
	uint8_t  int_ep;
	uint8_t  int_mps;
	uint8_t  int_interval;
	uint8_t  int_toggle;
	uint8_t  iface_number;
	uint8_t  parent_addr;
	uint8_t  parent_port;
} usb_device_t;

int usb_init(void);
int usb_control_msg(usb_device_t *dev, uint8_t rt, uint8_t req, uint16_t val, uint16_t idx, void *data, uint16_t len);
int usb_int_transfer(usb_device_t *dev, uint8_t ep, void *data, int len, int mps);
usb_device_t *usb_get_device(int index);
int usb_get_device_count(void);
usb_device_t *usb_alloc_device(void);
int usb_enumerate_device(usb_device_t *dev);
int usb_root_port_reset(void);
uint8_t usb_root_port_speed(void);
int usb_root_port_connected(void);

#endif
