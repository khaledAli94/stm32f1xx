#ifndef USB_DESC_H
#define USB_DESC_H

#include <stdint.h>

/* ---------- Standard USB descriptors ---------- */

struct usb_device_desc {
    uint8_t  bLength;             // Size of this descriptor (18 bytes)
    uint8_t  bDescriptorType;     // 0x01 for Device
    uint16_t bcdUSB;              // USB Spec Release (e.g., 0x0200 for USB 2.0)
    uint8_t  bDeviceClass;        // Class code (often 0x00, defined at Interface level)
    uint8_t  bDeviceSubClass;     // Subclass code
    uint8_t  bDeviceProtocol;     // Protocol code
    uint8_t  bMaxPacketSize0;     // EP0 max packet size (8, 16, 32, or 64)
    uint16_t idVendor;            // Vendor ID (assigned by USB-IF)
    uint16_t idProduct;           // Product ID (assigned by manufacturer)
    uint16_t bcdDevice;           // Device release number
    uint8_t  iManufacturer;       // Index of manufacturer string
    uint8_t  iProduct;            // Index of product string
    uint8_t  iSerialNumber;       // Index of serial number string
    uint8_t  bNumConfigurations;  // Number of possible configurations
} __attribute__((packed));
_Static_assert(sizeof(struct usb_device_desc) == 18, "USB device descriptor must be 18 bytes");

struct usb_configuration_desc {
    uint8_t  bLength;             /* Size of this descriptor: (9 bytes) */
    uint8_t  bDescriptorType;     /* USB_DESC_CONFIGURATION = 0x02 */
    uint16_t wTotalLength;        /* Total bytes for all configuration descriptors */
    uint8_t  bNumInterfaces;      /* Number of interfaces */
    uint8_t  bConfigurationValue; /* Value used by SET_CONFIGURATION */
    uint8_t  iConfiguration;      /* Configuration string index */
    uint8_t  bmAttributes;        /* Bus powered / self powered / remote wakeup */
    uint8_t  bMaxPower;           /* Maximum current in units of 2 mA */
} __attribute__((packed));
_Static_assert(sizeof(struct usb_configuration_desc) == 9, "USB configuration descriptor must be 9 bytes");

struct usb_interface_desc {
    uint8_t  bLength;             /* Size of this descriptor: (9 bytes) */
    uint8_t  bDescriptorType;     /* USB_DESC_INTERFACE = 0x04 */
    uint8_t  bInterfaceNumber;    /* Interface number */
    uint8_t  bAlternateSetting;   /* Alternate setting number */
    uint8_t  bNumEndpoints;       /* Number of endpoints excluding EP0 */
    uint8_t  bInterfaceClass;     /* USB class */
    uint8_t  bInterfaceSubClass;  /* USB subclass */
    uint8_t  bInterfaceProtocol;  /* USB protocol */
    uint8_t  iInterface;          /* Interface string index */
} __attribute__((packed));
_Static_assert(sizeof(struct usb_interface_desc) == 9, "USB interface descriptor must be 9 bytes");

struct usb_endpoint_desc {
    uint8_t  bLength;             // Size of this descriptor (7 bytes)
    uint8_t  bDescriptorType;     // 0x05 for Endpoint
    uint8_t  bEndpointAddress;    // Bit 7: Direction (1=IN, 0=OUT), Bits 3..0: EP Number
    uint8_t  bmAttributes;        // Transfer type (Control, Isochronous, Bulk, Interrupt)
    uint16_t wMaxPacketSize;      // Maximum packet size this endpoint can send/receive
    uint8_t  bInterval;           // Polling interval (used for Interrupt/Isochronous)
} __attribute__((packed));
_Static_assert(sizeof(struct usb_endpoint_desc) == 7, "USB endpoint descriptor must be 7 bytes");

struct usb_langid_string_desc {
    uint8_t  bLength;             // Size of descriptor (2 + length of string in bytes)
    uint8_t  bDescriptorType;     // 0x03 for String
    uint16_t bString[];           // UTF-16LE characters (flexible array member)
} __attribute__((packed));

#endif /* USB_DESC_H */
