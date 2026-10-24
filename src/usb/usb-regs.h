#ifndef USB_REGS_H
#define USB_REGS_H

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



/* BTABLE at PMA byte 0, 8 entries × 16 CPU bytes each = 128 CPU bytes.
 * Data buffers start at PMA byte 64 (CPU offset 128). */
#define USB_BD_AT(n)  ((struct usb_bdt_entry_t *)(USB_PMA_BASE + 16U * (n)))

/* 16-bit PMA access: byte offset b -> CPU addr USB_PMA_BASE + b */
#define PMA_BYTE(b)   (*(volatile unsigned short *)(USB_PMA_BASE + (b)))

/* PMA byte offsets of data buffers */
#define PMA_EP0_TX    64U
#define PMA_EP0_RX    128U
#define PMA_EP1_TX    192U
#define PMA_EP1_RX    256U
#define PMA_EP2_TX    320U

/* EPnR bits */
#define EP_EA        0x000FU
#define EP_STAT_TX   0x0030U
#define EP_DTOG_TX   0x0040U
#define EP_CTR_TX    0x0080U
#define EP_KIND      0x0100U
#define EP_TYPE      0x0600U
#define EP_SETUP     0x0800U
#define EP_STAT_RX   0x3000U
#define EP_DTOG_RX   0x4000U
#define EP_CTR_RX    0x8000U

#define EP_TYPE_BULK    0x0000U
#define EP_TYPE_CONTROL 0x0200U
#define EP_TYPE_INT     0x0600U

#define EP_DISABLED  0U
#define EP_STALL     1U
#define EP_NAK       2U
#define EP_VALID     3U

/* ISTR / CNTR bits */
#define ISTR_CTR     0x8000U
#define ISTR_RESET   0x0400U
#define ISTR_SUSP    0x0800U
#define ISTR_WKUP    0x1000U
#define ISTR_EP_ID   0x000FU
#define CNTR_FRES    0x0001U
#define CNTR_PDWN    0x0002U
#define CNTR_LPMODE  0x0004U
#define CNTR_FSUSP   0x0008U
#define CNTR_RESUME  0x0010U
#define CNTR_CTRM    0x8000U
#define CNTR_RESETM  0x0400U
#define CNTR_SUSPM   0x0800U
#define CNTR_WKUPM   0x1000U
#define CNTR_ERRM    0x2000U
#define CNTR_PMAOVRM 0x4000U

void usb_clock_init(void);
void usb_init(void);

#endif /* USB_REGS_H */
