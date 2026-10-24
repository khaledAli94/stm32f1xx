#ifndef USB_DESC_H
#define USB_DESC_H

struct usb_device_desc {
    unsigned char  bLength;             // Size of this descriptor (18 bytes)
    unsigned char  bDescriptorType;     // 0x01 for Device
    unsigned short bcdUSB;              // USB Spec Release (e.g., 0x0200 for USB 2.0)
    unsigned char  bDeviceClass;        // Class code (often 0x00, defined at Interface level)
    unsigned char  bDeviceSubClass;     // Subclass code
    unsigned char  bDeviceProtocol;     // Protocol code
    unsigned char  bMaxPacketSize0;     // EP0 max packet size (8, 16, 32, or 64)
    unsigned short idVendor;            // Vendor ID (assigned by USB-IF)
    unsigned short idProduct;           // Product ID (assigned by manufacturer)
    unsigned short bcdDevice;           // Device release number
    unsigned char  iManufacturer;       // Index of manufacturer string
    unsigned char  iProduct;            // Index of product string
    unsigned char  iSerialNumber;       // Index of serial number string
    unsigned char  bNumConfigurations;  // Number of possible configurations
} __attribute__((packed));

struct usb_endpoint_desc {
    unsigned char  bLength;             // Size of this descriptor (7 bytes)
    unsigned char  bDescriptorType;     // 0x05 for Endpoint
    unsigned char  bEndpointAddress;    // Bit 7: Direction (1=IN, 0=OUT), Bits 3..0: EP Number
    unsigned char  bmAttributes;        // Transfer type (Control, Isochronous, Bulk, Interrupt)
    unsigned short wMaxPacketSize;      // Maximum packet size this endpoint can send/receive
    unsigned char  bInterval;           // Polling interval (used for Interrupt/Isochronous)
} __attribute__((packed));

struct usb_string_desc {
    unsigned char  bLength;             // Size of descriptor (2 + length of string in bytes)
    unsigned char  bDescriptorType;     // 0x03 for String
    unsigned short bString[];           // UTF-16LE characters (flexible array member)
} __attribute__((packed));

#endif /* USB_DESC_H */
