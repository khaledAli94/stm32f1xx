#ifndef USB_REGS_H
#define USB_REGS_H

#include <stdint.h>

#define USB_BASE      0x40005C00UL
#define USB_PMA_BASE  0x40006000UL   // Packet memory area (shared with CAN) (512 bytes)

#define USB ((struct usb_regs_t *) USB_BASE)

struct usb_regs_t {
    volatile uint32_t EPR[8];      // 0x00-0x1C  Endpoint registers EP0R..EP7R
    volatile uint32_t RESERVED[8]; // 0x20-0x3C  Reserved
    volatile uint32_t CNTR;        // 0x40 Control register
    volatile uint32_t ISTR;        // 0x44 Interrupt status register
    volatile uint32_t FNR;         // 0x48 Frame number register
    volatile uint32_t DADDR;       // 0x4C Device address register
    volatile uint32_t BTABLE;      // 0x50 Buffer table address register
};

/*
 * Single-buffer:
 *   ADDR_TX/COUNT_TX = TX buffer
 *   ADDR_RX/COUNT_RX = RX buffer
 *
 * Double-buffer:
 *   ADDR_TX/COUNT_TX = Buffer 0
 *   ADDR_RX/COUNT_RX = Buffer 1
 */

/* Buffer Descriptor Table (BDT) 4 half-words */
struct usb_bdt_entry_t {
    volatile uint16_t ADDR_TX;   // 0x0 Transmission buffer address
    uint16_t PAD0;
    volatile uint16_t COUNT_TX;  // 0x4 Transmission byte count
    uint16_t PAD1;
    volatile uint16_t ADDR_RX;   // 0x8 Reception buffer address
    uint16_t PAD2;
    volatile uint16_t COUNT_RX;  // 0xc Reception byte count
    uint16_t PAD3;
};
/*
 * BDT assumed at PMA offset 0 (USB->BTABLE = 0).
 * If BTABLE is relocated, USB_BDT must be adjusted.
 *  the USB PMA is 512 bytes, organised as 256 half-words, 
 * but the CPU does not see it as a contiguous byte array. 
 * Each 16-bit PMA half-word is mapped to a 32-bit slot on the CPU bus 
 * The BDT (8 entries x 8 bytes) occupies PMA bytes 0..63
 */


/* BTABLE at PMA byte 0, 8 entries × 16 CPU bytes each = 128 CPU bytes.
 * Data buffers start at PMA byte 64 (CPU offset 128). */
#define USB_BD_AT(n)  ((struct usb_bdt_entry_t *)(USB_PMA_BASE + 16U * (n)))

/* 16-bit PMA access: byte offset b -> CPU addr USB_PMA_BASE + 2b */
#define PMA_BYTE(b)   (*(volatile uint16_t *)(USB_PMA_BASE + 2U * (b)))

/* PMA byte offsets of data buffers (must start >= 128 to avoid BDT overlap at 0-127) */
#define PMA_EP0_TX    64U 
#define PMA_EP0_RX    128U
#define PMA_EP1_TX    192U
#define PMA_EP1_RX    256U
#define PMA_EP2_TX    320U



/* EPnR  reg */
#define EP_ADDR_POS      0U
#define EP_STAT_TX_POS   4U
#define EP_TYPE_POS      9U
#define EP_STAT_RX_POS   12U

#define EP_DTOG_TX       (1U << 6)
#define EP_CTR_TX        (1U << 7)
#define EP_KIND          (1U << 8)
#define EP_SETUP         (1U << 11)
#define EP_DTOG_RX       (1U << 14)
#define EP_CTR_RX        (1U << 15)

#define EP_ADDR_MASK     (0xfU << EP_ADDR_POS)
#define EP_STAT_TX_MASK  (0x3U << EP_STAT_TX_POS)
#define EP_TYPE_MASK     (0x3U << EP_TYPE_POS)
#define EP_STAT_RX_MASK  (0x3U << EP_STAT_RX_POS)

/* endpoint address encoding */
#define EP_ADDR(x)       ((x) << EP_ADDR_POS)

/* endpoint status encoding */
#define EP_TX_STAT(x)    ((x) << EP_STAT_TX_POS)
#define EP_RX_STAT(x)    ((x) << EP_STAT_RX_POS)

/* endpoint types */
enum {
    EP_TYPE_BULK    = (0U << EP_TYPE_POS),
    EP_TYPE_CONTROL = (1U << EP_TYPE_POS),
    EP_TYPE_ISO     = (2U << EP_TYPE_POS),
    EP_TYPE_INT     = (3U << EP_TYPE_POS)
};

/* Endpoint status */
enum {
    EP_DISABLED = 0,
    EP_STALL,
    EP_NAK,
    EP_VALID
};  

/* ---------- BDT COUNT_RX ---------- */

#define COUNT_NBYTES_POS         0U
#define COUNT_NBYTES_MASK        0x03FFU

#define COUNT_RX_NUM_BLOCK_POS   10U
#define COUNT_RX_NUM_BLOCK_MASK  (0x1FU << COUNT_RX_NUM_BLOCK_POS)

#define COUNT_RX_BLSIZE_POS      15U
#define COUNT_RX_BLSIZE_MASK     (1U << COUNT_RX_BLSIZE_POS)

/* 64-byte RX buffer */
#define COUNT_RX_64B ((1U << COUNT_RX_BLSIZE_POS) | (1U << COUNT_RX_NUM_BLOCK_POS))


/* ISTR bits */
#define ISTR_EP_ID      (0xfU << 0)
#define ISTR_ESOF       (0x1U << 8)
#define ISTR_SOF        (0x1U << 9)
#define ISTR_RESET      (0x1U << 10)
#define ISTR_SUSP       (0x1U << 11)
#define ISTR_WKUP       (0x1U << 12)
#define ISTR_CTR        (0x1U << 15) /* Correct transfer */

/* CNTR bits */
#define CNTR_FRES       (0x1U << 0) 
#define CNTR_PDWN       (0x1U << 1) 
#define CNTR_LPMODE     (0x1U << 2) 
#define CNTR_FSUSP      (0x1U << 3) 
#define CNTR_RESUME     (0x1U << 4) 
#define CNTR_RESETM     (0x1U << 10)
#define CNTR_SUSPM      (0x1U << 11)
#define CNTR_WKUPM      (0x1U << 12)
#define CNTR_ERRM       (0x1U << 13)
#define CNTR_PMAOVRM    (0x1U << 14)
#define CNTR_CTRM       (0x1U << 15)

#define DADDR_EF        (0x1U << 7) /* en function */

#endif /* USB_REGS_H */
