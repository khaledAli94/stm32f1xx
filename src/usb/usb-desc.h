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



static uint8_t lc[7] = { 
    0x00, 0xC2, 0x01, 0x00,  // 115200
    0x00, // 1 stop bit 
    0x00, // parity none
    0x08 // data bits
}; // Line Coding

/* ---------- descriptors ---------- */
static const uint8_t devd[] = {
    18,       // bLength
    0x01,     // DEVICE descriptor
    0x00, 0x02, // USB 2.0
    0x02,     // Device class
    0x00,     // Subclass
    0x00,     // Protocol
    64,       // EP0 max packet size
    0x83, 0x04, // VID
    0x40, 0x57, // PID
    0x00, 0x01, // Device release
    0,        // Manufacturer string
    0,        // Product string
    0,        // Serial number string
    1         // Number of configurations
};
_Static_assert(sizeof(devd) == 18, "dev descriptor size drift");

static const uint8_t cfgd[] = {
    /* Configuration Descriptor */
    9,          // bLength
    0x02,       // CONFIGURATION descriptor
    0x43, 0x00, // wTotalLength = 67 bytes
    2,          // bNumInterfaces
    1,          // bConfigurationValue
    0,          // iConfiguration
    0x80,       // bmAttributes: Bus-powered, no remote wakeup
    0x32,       // bMaxPower: 100mA

    /* Interface 0: CDC Communication Interface */
    9,          // bLength
    0x04,       // INTERFACE descriptor
    0,          // bInterfaceNumber
    0,          // bAlternateSetting
    1,          // bNumEndpoints
    0x02,       // bInterfaceClass: CDC Communication
    0x02,       // bInterfaceSubClass: Abstract Control Model
    0x01,       // bInterfaceProtocol: AT commands
    0,          // iInterface

    /* CDC Header Functional Descriptor */
    5,          // bLength
    0x24,       // CS_INTERFACE
    0x00,       // Header Functional Descriptor
    0x10, 0x01, // bcdCDC: CDC 1.10

    /* CDC Call Management Functional Descriptor */
    5,          // bLength
    0x24,       // CS_INTERFACE
    0x01,       // Call Management Functional Descriptor
    0x00,       // bmCapabilities
    0x01,       // bDataInterface: Interface 1

    /* CDC Abstract Control Management (ACM) Descriptor */
    4,          // bLength
    0x24,       // CS_INTERFACE
    0x02,       // ACM Functional Descriptor
    0x02,       // bmCapabilities

    /* CDC Union Functional Descriptor */
    5,          // bLength
    0x24,       // CS_INTERFACE
    0x06,       // Union Functional Descriptor
    0x00,       // bMasterInterface: Interface 0
    0x01,       // bSlaveInterface0: Interface 1

    /* Endpoint 2 IN: Interrupt */
    7,          // bLength
    0x05,       // ENDPOINT descriptor
    0x82,       // bEndpointAddress: EP2 IN
    0x03,       // bmAttributes: Interrupt
    0x08, 0x00, // wMaxPacketSize: 8 bytes
    0x0A,       // bInterval: 10

    /* Interface 1: CDC Data Interface */
    9,          // bLength
    0x04,       // INTERFACE descriptor
    1,          // bInterfaceNumber
    0,          // bAlternateSetting
    2,          // bNumEndpoints
    0x0A,       // bInterfaceClass: CDC Data
    0x00,       // bInterfaceSubClass
    0x00,       // bInterfaceProtocol
    0,          // iInterface

    /* Endpoint 1 Bulk IN */
    7,          // bLength
    0x05,       // ENDPOINT descriptor
    0x81,       // bEndpointAddress: EP1 IN
    0x02,       // bmAttributes: Bulk
    0x40, 0x00, // wMaxPacketSize: 64 bytes
    0x00,       // bInterval

    /* Endpoint 1: Bulk OUT */
    7,          // bLength
    0x05,       // ENDPOINT descriptor
    0x01,       // bEndpointAddress: EP1 OUT
    0x02,       // bmAttributes: Bulk
    0x40, 0x00, // wMaxPacketSize: 64 bytes
    0x00        // bInterval
};
_Static_assert(sizeof(cfgd) == 67, "cfgd descriptor size drift");

static const uint8_t str0[] = { 4, 0x03, 0x09, 0x04 }; // supported language id 



#endif /* USB_DESC_H */
