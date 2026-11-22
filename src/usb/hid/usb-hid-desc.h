#ifndef USB_HID_DESC_H
#define USB_HID_DESC_H

#include <usb-desc.h>

/* ---------- HID class-specific functional descriptors ---------- */

#define HID_MOUSE_REPORT_ID        1U
#define HID_MOUSE_REPORT_SIZE      6U

#define HID_MOUSE_BUTTON_LEFT      0x01U
#define HID_MOUSE_BUTTON_RIGHT     0x02U
#define HID_MOUSE_BUTTON_MIDDLE    0x04U

#define HID_KEYBOARD_REPORT_ID     2U
#define HID_KEYBOARD_REPORT_SIZE   9U

#define HID_KEY_MOD_LEFT_CTRL      0x01U


static const uint8_t hid_report_desc[] = {
    /* ---------- Report ID 1: Mouse ---------- */
    0x05, 0x01,                         /* Usage Page (Generic Desktop) */
    0x09, 0x02,                         /* Usage (Mouse) */
    0xA1, 0x01,                         /* Collection (Application) */

    0x85, HID_MOUSE_REPORT_ID,          /*   Report ID (1) */

    0x09, 0x01,                         /*   Usage (Pointer) */
    0xA1, 0x00,                         /*   Collection (Physical) */

    0x05, 0x09,                         /*     Usage Page (Button) */
    0x19, 0x01,                         /*     Usage Minimum (Button 1) */
    0x29, 0x03,                         /*     Usage Maximum (Button 3) */
    0x15, 0x00,                         /*     Logical Minimum (0) */
    0x25, 0x01,                         /*     Logical Maximum (1) */
    0x75, 0x01,                         /*     Report Size (1) */
    0x95, 0x03,                         /*     Report Count (3) */
    0x81, 0x02,                         /*     Input (Data,Var,Abs) */

    0x75, 0x05,                         /*     Report Size (5) */
    0x95, 0x01,                         /*     Report Count (1) */
    0x81, 0x03,                         /*     Input (Const,Var,Abs), padding */

    0x05, 0x01,                         /*     Usage Page (Generic Desktop) */
    0x09, 0x30,                         /*     Usage (X) */
    0x09, 0x31,                         /*     Usage (Y) */
    0x09, 0x38,                         /*     Usage (Wheel) */
    0x15, 0x81,                         /*     Logical Minimum (-127) */
    0x25, 0x7F,                         /*     Logical Maximum (127) */
    0x75, 0x08,                         /*     Report Size (8) */
    0x95, 0x03,                         /*     Report Count (3) */
    0x81, 0x06,                         /*     Input (Data,Var,Rel) */

    0x05, 0x0C,                         /*     Usage Page (Consumer Devices) */
    0x0A, 0x38, 0x02,                   /*     Usage (AC Pan) */
    0x15, 0x81,                         /*     Logical Minimum (-127) */
    0x25, 0x7F,                         /*     Logical Maximum (127) */
    0x75, 0x08,                         /*     Report Size (8) */
    0x95, 0x01,                         /*     Report Count (1) */
    0x81, 0x06,                         /*     Input (Data,Var,Rel) */

    0xC0,                               /*   End Collection */
    0xC0,                               /* End Collection */

    /* ---------- Report ID 2: Keyboard ---------- */
    0x05, 0x01,                         /* Usage Page (Generic Desktop) */
    0x09, 0x06,                         /* Usage (Keyboard) */
    0xA1, 0x01,                         /* Collection (Application) */

    0x85, HID_KEYBOARD_REPORT_ID,       /*   Report ID (2) */

    0x05, 0x07,                         /*   Usage Page (Keyboard/Keypad) */
    0x19, 0xE0,                         /*   Usage Minimum (Keyboard LeftControl) */
    0x29, 0xE7,                         /*   Usage Maximum (Keyboard Right GUI) */
    0x15, 0x00,                         /*   Logical Minimum (0) */
    0x25, 0x01,                         /*   Logical Maximum (1) */
    0x75, 0x01,                         /*   Report Size (1) */
    0x95, 0x08,                         /*   Report Count (8) */
    0x81, 0x02,                         /*   Input (Data,Var,Abs) */

    0x75, 0x08,                         /*   Report Size (8) */
    0x95, 0x01,                         /*   Report Count (1) */
    0x81, 0x03,                         /*   Input (Const,Var,Abs), reserved */

    0x19, 0x00,                         /*   Usage Minimum (0) */
    0x29, 0x65,                         /*   Usage Maximum (Keyboard Application) */
    0x15, 0x00,                         /*   Logical Minimum (0) */
    0x25, 0x65,                         /*   Logical Maximum (101) */
    0x75, 0x08,                         /*   Report Size (8) */
    0x95, 0x06,                         /*   Report Count (6) */
    0x81, 0x00,                         /*   Input (Data,Array,Abs) */

    0xC0                                /* End Collection */
};


static const uint8_t devd[] = {
    18,                                 /* bLength */
    0x01,                               /* bDescriptorType: Device */
    0x00, 0x02,                         /* bcdUSB: USB 2.00 */
    0x00,                               /* bDeviceClass */
    0x00,                               /* bDeviceSubClass */
    0x00,                               /* bDeviceProtocol */
    64,                                 /* bMaxPacketSize0 */
    0x34, 0x12,                         /* idVendor - replace for real product */
    0x78, 0x56,                         /* idProduct - replace for real product */
    0x00, 0x01,                         /* bcdDevice */
    0x00,                               /* iManufacturer */
    0x00,                               /* iProduct */
    0x00,                               /* iSerialNumber */
    0x01                                /* bNumConfigurations */
};

static const uint8_t cfgd[] = {
    /* Configuration descriptor */
    0x09,
    0x02,
    0x22, 0x00,                         /* wTotalLength = 34 */
    0x01,                               /* bNumInterfaces */
    0x01,                               /* bConfigurationValue */
    0x00,                               /* iConfiguration */
    0x80,                               /* bus powered */
    0x32,                               /* 100 mA */

    /* Interface descriptor */
    0x09,
    0x04,
    0x00,                               /* bInterfaceNumber */
    0x00,                               /* bAlternateSetting */
    0x01,                               /* bNumEndpoints */
    0x03,                               /* bInterfaceClass: HID */
    0x00,                               /* bInterfaceSubClass */
    0x00,                               /* bInterfaceProtocol */
    0x00,                               /* iInterface */

    /* HID descriptor */
    0x09,
    0x21,                               /* HID descriptor type */
    0x11, 0x01,                         /* HID version 1.11 */
    0x00,                               /* country code */
    0x01,                               /* one subordinate descriptor */
    0x22,                               /* Report descriptor */
    (uint8_t)(sizeof(hid_report_desc) & 0xFFU),
    (uint8_t)(sizeof(hid_report_desc) >> 8),

    /* Endpoint descriptor */
    0x07,
    0x05,
    0x81,                               /* IN endpoint 1 */
    0x03,                               /* Interrupt endpoint */
    0x40, 0x00,                         /* wMaxPacketSize = 64 */
    0x01                                /* poll interval = 1 ms */
};


/* USB language-ID descriptor: English (United States). */
static const uint8_t str0[] = {
    0x04,
    0x03,
    0x09, 0x04
};

#undef HID_CONTACT_COLLECTION

#endif /* USB_HID_DESC_H */
