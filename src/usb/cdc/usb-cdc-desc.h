#ifndef USB_CDC_DESC_H
#define USB_CDC_DESC_H

#include <usb-desc.h>

/* ---------- CDC class-specific functional descriptors ---------- */

struct cdc_header_functional_desc {
    uint8_t  bFunctionLength;     /* Size of this descriptor: 5 */
    uint8_t  bDescriptorType;     /* CS_INTERFACE = 0x24 */
    uint8_t  bDescriptorSubType;  /* Header functional descriptor = 0x00 */
    uint16_t bcdCDC;              /* CDC specification version */
} __attribute__((packed));

struct cdc_call_management_functional_desc {
    uint8_t bFunctionLength;      /* Size of this descriptor: 5 */
    uint8_t bDescriptorType;      /* CS_INTERFACE = 0x24 */
    uint8_t bDescriptorSubType;   /* Call-management descriptor = 0x01 */
    uint8_t bmCapabilities;       /* Call-management capability flags */
    uint8_t bDataInterface;       /* Interface number of CDC data interface */
} __attribute__((packed));

struct cdc_acm_functional_desc {
    uint8_t bFunctionLength;      /* Size of this descriptor: 4 */
    uint8_t bDescriptorType;      /* CS_INTERFACE = 0x24 */
    uint8_t bDescriptorSubType;   /* ACM descriptor = 0x02 */
    uint8_t bmCapabilities;       /* ACM capability flags */
} __attribute__((packed));

struct cdc_union_functional_desc {
    uint8_t bFunctionLength;      /* Size of this descriptor: 5 */
    uint8_t bDescriptorType;      /* CS_INTERFACE = 0x24 */
    uint8_t bDescriptorSubType;   /* Union descriptor = 0x06 */
    uint8_t bMasterInterface;     /* CDC communication interface number */
    uint8_t bSlaveInterface0;     /* CDC data interface number */
} __attribute__((packed));

/* combined cdc cfg descriptor  (oder matters)*/
struct cdc_acm_configuration_desc {
    struct usb_configuration_desc configuration;

    struct usb_interface_desc communication_iface;
    struct cdc_header_functional_desc hdr;
    struct cdc_call_management_functional_desc call_management;
    struct cdc_acm_functional_desc acm;
    struct cdc_union_functional_desc union_desc;
    struct usb_endpoint_desc notification_endpoint;

    struct usb_interface_desc data_iface;
    struct usb_endpoint_desc bulk_in_ep;
    struct usb_endpoint_desc bulk_out_ep;
} __attribute__((packed));


struct cdc_line_coding {
    uint32_t dwDTERate;           /* Bit rate in bits per second */
    uint8_t  bCharFormat;         /* 0=1 stop, 1=1.5 stop, 2=2 stop bits */
    uint8_t  bParityType;         /* 0=none, 1=odd, 2=even, etc. */
    uint8_t  bDataBits;           /* Usually 5, 6, 7, 8, or 16 */
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
_Static_assert(sizeof(devd) == 18, "dev descriptor must be 18 byte");

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
_Static_assert(sizeof(cfgd) == 67, "cfgd descriptor must be 67 byte");

static const uint8_t str0[] = { 4, 0x03, 0x09, 0x04 }; // supported language id 


#endif /* USB_CDC_DESC_H */
