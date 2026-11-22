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

/* combined cdc cfg descriptor  */

struct cdc_acm_configuration_desc {
    struct usb_configuration_desc configuration;

    struct usb_interface_desc     communication_iface;
    struct cdc_header_functional_desc hdr;
    struct cdc_call_management_functional_desc call_management;
    struct cdc_acm_functional_desc acm;
    struct cdc_union_functional_desc union_desc;
    struct usb_endpoint_desc      notification_endpoint;

    struct usb_interface_desc     data_iface;
    struct usb_endpoint_desc      bulk_in_ep;
    struct usb_endpoint_desc      bulk_out_ep;
} __attribute__((packed));


struct cdc_line_coding {
    uint32_t dwDTERate;           /* Bit rate in bits per second */
    uint8_t  bCharFormat;         /* 0=1 stop, 1=1.5 stop, 2=2 stop bits */
    uint8_t  bParityType;         /* 0=none, 1=odd, 2=even, etc. */
    uint8_t  bDataBits;           /* Usually 5, 6, 7, 8, or 16 */
} __attribute__((packed));


#endif /* USB_CDC_DESC_H */
