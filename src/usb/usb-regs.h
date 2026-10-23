
/*
 * BDT assumed at PMA offset 0 (USB->BTABLE = 0).
 * If BTABLE is relocated, USB_BDT must be adjusted.
 */

#define USB_BASE      0x40005C00UL
#define USB_PMA_BASE  0x40006000UL   // Packet memory area (shared with CAN) (512 bytes)

#define USB ((struct usb_regs_t *) USB_BASE)

struct usb_regs_t {
    volatile unsigned EPR[8];      // 0x00-0x1C  Endpoint registers EP0R..EP7R
    volatile unsigned RESERVED[8]; // 0x20-0x3C  Reserved
    volatile unsigned CNTR;        // 0x40 Control register
    volatile unsigned ISTR;        // 0x44 Interrupt status register
    volatile unsigned FNR;         // 0x48 Frame number register
    volatile unsigned DADDR;       // 0x4C Device address register
    volatile unsigned BTABLE;      // 0x50 Buffer table address register
};

/* Buffer Descriptor Table (BDT) 4 half-words */
struct usb_bdt_entry_t {
    volatile unsigned short ADDR_TX;   // 0x0 Transmission buffer address
    unsigned short PAD0;
    volatile unsigned short COUNT_TX;  // 0x4 Transmission byte count
    unsigned short PAD1;
    volatile unsigned short ADDR_RX;   // 0x8 Reception buffer address
    unsigned short PAD2;
    volatile unsigned short COUNT_RX;  // 0xc Reception byte count
    unsigned short PAD3;
};



/* USB endpoint-register bits */
#define USB_EP_EA           0x000fU
#define USB_EP_STAT_TX      0x0030U
#define USB_EP_DTOG_TX      0x0040U
#define USB_EP_CTR_TX       0x0080U
#define USB_EP_KIND         0x0100U
#define USB_EP_TYPE         0x0600U
#define USB_EP_SETUP        0x0800U
#define USB_EP_STAT_RX      0x3000U
#define USB_EP_DTOG_RX      0x4000U
#define USB_EP_CTR_RX       0x8000U

#define USB_EP_TYPE_BULK    0x0000U
#define USB_EP_TYPE_CONTROL 0x0200U
#define USB_EP_TYPE_INT     0x0600U

#define USB_EP_DISABLED     0U
#define USB_EP_STALL        1U
#define USB_EP_NAK          2U
#define USB_EP_VALID        3U