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


static unsigned char  lc[7] = { 
    0x00, 0xC2, 0x01, 0x00,  // 115200
    0x00, // 1 stop bit 
    0x00, // parity none
    0x08 // data bits
}; // Line Coding

/* ---------- descriptors ---------- */
static const unsigned char devd[] = {
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

static const unsigned char cfgd[] = {
    /* Configuration Descriptor */
    9,          // bLength
    0x02,       // CONFIGURATION descriptor
    0x43, 0x00, // wTotalLength = 67 bytes
    2,          // bNumInterfaces
    1,          // bConfigurationValue
    0,          // iConfiguration
    0x80,       // bmAttributes: Bus-powered, no remote wakeup
    0x32,       // bMaxPower: 100mA

    /* Interface 0: CDC Communication */
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

    /* Interface 1: CDC Data */
    9,          // bLength
    0x04,       // INTERFACE descriptor
    1,          // bInterfaceNumber
    0,          // bAlternateSetting
    2,          // bNumEndpoints
    0x0A,       // bInterfaceClass: CDC Data
    0x00,       // bInterfaceSubClass
    0x00,       // bInterfaceProtocol
    0,          // iInterface

    /* Endpoint 1 IN: Bulk */
    7,          // bLength
    0x05,       // ENDPOINT descriptor
    0x81,       // bEndpointAddress: EP1 IN
    0x02,       // bmAttributes: Bulk
    0x40, 0x00, // wMaxPacketSize: 64 bytes
    0x00,       // bInterval

    /* Endpoint 1 OUT: Bulk */
    7,          // bLength
    0x05,       // ENDPOINT descriptor
    0x01,       // bEndpointAddress: EP1 OUT
    0x02,       // bmAttributes: Bulk
    0x40, 0x00, // wMaxPacketSize: 64 bytes
    0x00        // bInterval
};
_Static_assert(sizeof(cfgd) == 67, "cfgd descriptor size drift");

static const unsigned char str0[] = { 4, 0x03, 0x09, 0x04 };



#endif /* USB_DESC_H */
