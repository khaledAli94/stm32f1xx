#ifndef USB_CORE_H
#define USB_CORE_H

#include <usb-regs.h>
#include <usb-desc.h>

#include <clock.h>
#include <gpio.h>
#include <nested-vector.h>
#include <flash-regs.h>


enum usb_ctrl_state_t {
    CTRL_IDLE        = 0,
    CTRL_DATA_OUT    = 1,
    CTRL_SET_ADDRESS = 2,
    CTRL_DATA_IN     = 3,
    CTRL_STATUS_IN   = 5,
    CTRL_STATUS_OUT  = 6
};

enum {
    REQ_GET_STATUS        = 0x0,
    REQ_CLEAR_FEATURE     = 0x1,
    REQ_SET_FEATURE       = 0x3,
    REQ_SET_ADDRESS       = 0x5,
    REQ_GET_DESCRIPTOR    = 0x6,
    REQ_SET_DESCRIPTOR    = 0x7,
    REQ_GET_CONFIGURATION = 0x8,
    REQ_SET_CONFIGURATION = 0x9,
    REQ_GET_INTERFACE     = 0xA,
    REQ_SET_INTERFACE     = 0xB,
};

enum {
    CDC_SET_LINE_CODING        = 0x20,
    CDC_GET_LINE_CODING        = 0x21,
    CDC_SET_CONTROL_LINE_STATE = 0x22,
    CDC_SEND_BREAK             = 0x23,
};

enum {
    USB_DESC_DEVICE        = 0x1,
    USB_DESC_CONFIGURATION = 0x2,
    USB_DESC_STRING        = 0x3,
    USB_DESC_INTERFACE     = 0x4,
    USB_DESC_ENDPOINT      = 0x5,
};

enum {
    USB_REQ_STANDARD = 0x0,
    USB_REQ_CLASS    = 0x20,
    USB_REQ_VENDOR   = 0x40,
};


enum {
    CDC_EP_DATA_OUT = 0x1,
    CDC_EP_DATA_IN  = 0x81,
    CDC_EP_COMM_IN  = 0x82,
};

void usb_clock_init(void);
void usb_init(void);


#endif /* USB_CORE_H */
