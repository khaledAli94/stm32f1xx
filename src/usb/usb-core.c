#include <clock.h>
#include <usb-regs.h>
#include <gpio.h>
#include <nested-vector.h>
#include <flash-regs.h>

/*
* per modemax bulk packet sz
* Low-Speed  1.5 Mbit/s 8 bytes
* Full-Speed 12 Mbit/s 64 bytes
* High-Speed 480 Mbit/s 512 bytes
*/

enum ctrl_state_t {
    CTRL_IDLE        = 0,
    CTRL_DATA_OUT    = 1,
    CTRL_SET_ADDRESS = 2,
    CTRL_DATA_IN     = 3,
    CTRL_STATUS_IN   = 5,
    CTRL_STATUS_OUT  = 6
};

static unsigned char    usb_addr;
static enum ctrl_state_t  ctrl_pending;
static unsigned char    ctrl_buf[128];
static unsigned         ctrl_len;
static unsigned         ctrl_pos;

static unsigned char  bulk_tx_busy;
static unsigned char  bulk_tx_buf[64];
static unsigned       bulk_tx_len;

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

/* ---------- clock, using the enums from clock.h ---------- */
void usb_clock_init(void) {
    /* Flash: 2 wait states + prefetch (needed for 72 MHz on F103) */
    flash_set_latency(72000000);

    RCC->CR |= (1U << 16);                       /* HSEON  */
    while (!(RCC->CR & (1U << 17))) { }           /* HSERDY */

    RCC->CFGR = ((unsigned)CLOCK_PLL   << 0)
              | ((unsigned)AHB_DIV_1   << 4)
              | ((unsigned)APB_DIV_2   << 8)
              | ((unsigned)APB_DIV_1   << 11)
              | (1U << 16)                       /* PLLSRC = HSE */
              | ((unsigned)PLL_MUL_9   << 18)
              | ((unsigned)USB_DIV_1_5 << 22);

    RCC->CR |= (1U << 24);                       /* PLLON  */
    while (!(RCC->CR & (1U << 25))) { }           /* PLLRDY */

    RCC->CFGR = (RCC->CFGR & ~0x3U) | CLOCK_PLL; /* switch SYSCLK */
    while (((RCC->CFGR >> 2) & 0x3U) != CLOCK_PLL) { }
}

/* ---------- PMA helpers (16-bit only, 2:1 hole) ---------- */
static void pma_write16(unsigned b, const unsigned char *s, unsigned n) {
    unsigned i;
    for (i = 0; i < n; i += 2) {
        unsigned short w = s[i];
        if (i + 1 < n) w |= (unsigned short)s[i + 1] << 8;
        PMA_BYTE(b + i) = w;
    }
}
static void pma_read16(unsigned b, unsigned char *d, unsigned n) {
    unsigned i;
    for (i = 0; i < n; i += 2) {
        unsigned short w = PMA_BYTE(b + i);
        d[i] = (unsigned char)(w & 0xFF);
        if (i + 1 < n) d[i + 1] = (unsigned char)(w >> 8);
    }
}

/* ---------- EPnR and packet transfer events handlers --------- */
static inline void ep_set(unsigned ep, unsigned mask, unsigned data) {
    unsigned old = USB->EPR[ep];
    unsigned rc_w0  = (EP_CTR_RX | EP_CTR_TX) ;
    unsigned toggle = (EP_DTOG_RX | EP_STAT_RX | EP_DTOG_TX | EP_STAT_TX);
    unsigned rw     = (EP_TYPE | EP_KIND | EP_EA);

    unsigned wr0 = rc_w0 & (~mask | data);
    unsigned wr1 = (mask & toggle) & (old ^ data);
    unsigned wr2 = rw & ((old & ~mask) | data);

    USB->EPR[ep] = (unsigned short)(wr0 | wr1 | wr2);
}

static void ep_set_tx(unsigned ep, unsigned s) {
    ep_set(ep, 0x0030U, s << 4);
}
static void ep_set_rx(unsigned ep, unsigned s) {
    if (ep == 0) USB_BD_AT(0)->COUNT_RX = 0x8400;
    else if (ep == 1) USB_BD_AT(1)->COUNT_RX = 0x8400;
    ep_set(ep, 0x3000U, s << 12);
}
static void ep_clear_ctr_rx(unsigned ep) {
    ep_set(ep, 0x8000U, 0);
}
static void ep_clear_ctr_tx(unsigned ep) {
    ep_set(ep, 0x0080U, 0);
}

/* ---------- init ---------- */
void usb_init(void) {
    unsigned d;

    RCC->APB2ENR |= (1U << 2);                   /* GPIOA-EN */
    RCC->APB1ENR |= (1U << 23);                  /* USB-EN  */

    /* PA11 / PA12 as input floating */
    GPIOA->CRH = (GPIOA->CRH & ~0x000FF000U) | 0x00044000U;

    for (d = 0x40000; d; --d) { }                /* settle */

    USB->CNTR = CNTR_FRES;
    for (d = 0x40000; d; --d) { }
    USB->CNTR = 0;
    USB->ISTR = 0;
    USB->BTABLE = 0;                             /* BTABLE at PMA byte 0 */

    /* EP0: control, RX valid, TX NAK */
    ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | (EP_NAK << 4) | (EP_VALID << 12));
    USB_BD_AT(0)->ADDR_RX  = PMA_EP0_RX;  USB_BD_AT(0)->COUNT_RX = 0x8400;
    USB_BD_AT(0)->ADDR_TX  = PMA_EP0_TX;  USB_BD_AT(0)->COUNT_TX = 0;

    USB->CNTR  = CNTR_CTRM | CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
    NVIC->ISER[0] = (1U << 20);   /* NVIC USB_LP */
}

/* ---------- control-transfer helpers ---------- */
static void ep0_send_chunk(void) {
    unsigned n = ctrl_len - ctrl_pos;
    if (n > 64) n = 64;
    pma_write16(PMA_EP0_TX, ctrl_buf + ctrl_pos, n);
    USB_BD_AT(0)->COUNT_TX = (unsigned short)n;
    ctrl_pos += n;
    ctrl_pending = CTRL_DATA_IN;
    ep_set_tx(0, EP_VALID);
}

static void ep0_setup(void) {
    unsigned char s[8];
    unsigned bmReq, bReq, wVal, wLen;

    pma_read16(PMA_EP0_RX, s, 8);
    ep_clear_ctr_rx(0);
    bmReq = s[0]; bReq = s[1];
    wVal  = s[2] | ((unsigned)s[3] << 8);
    wLen  = s[6] | ((unsigned)s[7] << 8);

    ctrl_len = 0; ctrl_pos = 0; ctrl_pending = CTRL_IDLE;

    if ((bmReq & 0x60U) == 0x00U) {                    /* standard */
        if (bReq == 0x06U) {                            /* GET_DESCRIPTOR */
            unsigned type = wVal >> 8;
            const unsigned char *src = 0; unsigned n = 0;
            if      (type == 0x01U) { src = devd; n = sizeof(devd); }
            else if (type == 0x02U) { src = cfgd; n = sizeof(cfgd); }
            else if (type == 0x03U && (wVal & 0xFFU) == 0) { src = str0; n = sizeof(str0);}
            if (!src) { ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL); return; }
            if (n > sizeof(ctrl_buf)) n = sizeof(ctrl_buf);
            if (n > wLen) n = wLen;
            { unsigned i; for (i = 0; i < n; i++) ctrl_buf[i] = src[i]; }
            ctrl_len = n;
            ep0_send_chunk();
        } else if (bReq == 0x05U) {                     /* SET_ADDRESS */
            usb_addr = (unsigned char)(wVal & 0x7FU);
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_SET_ADDRESS;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x09U) {                     /* SET_CONFIGURATION */
            if (wVal) {
                USB_BD_AT(1)->ADDR_TX = PMA_EP1_TX;  USB_BD_AT(1)->COUNT_TX = 0;
                USB_BD_AT(1)->ADDR_RX = PMA_EP1_RX;  USB_BD_AT(1)->COUNT_RX = 0x8400;
                ep_set(1, 0xFFFFU, 0x0001U | EP_TYPE_BULK
                                 | (EP_NAK << 4) | (EP_VALID << 12));

                USB_BD_AT(2)->ADDR_TX = PMA_EP2_TX;  USB_BD_AT(2)->COUNT_TX = 0;
                ep_set(2, 0xFFFFU, 0x0002U | EP_TYPE_INT
                                 | (EP_NAK << 4) | (EP_DISABLED << 12));
            }
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x00U) {                     /* GET_STATUS */
            ctrl_buf[0] = 0; ctrl_buf[1] = 0;
            ctrl_len = 2;
            ep0_send_chunk();
        } else if (bReq == 0x01U || bReq == 0x03U) {    /* CLEAR_FEATURE / SET_FEATURE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x0BU) {                     /* SET_INTERFACE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x0AU) {                     /* GET_INTERFACE */
            ctrl_buf[0] = 0;
            ctrl_len = 1;
            ep0_send_chunk();
        } else {
            ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL);
        }
    } else if ((bmReq & 0x60U) == 0x20U) {              /* CDC class */
        if (bReq == 0x20U) {                            /* SET_LINE_CODING */
            ctrl_len = (wLen < 7) ? wLen : 7;
            ctrl_pos = 0;
            ctrl_pending = CTRL_DATA_OUT;
            ep_set_rx(0, EP_VALID);
        } else if (bReq == 0x21U) {                     /* GET_LINE_CODING */
            unsigned i; for (i = 0; i < 7; i++) ctrl_buf[i] = lc[i];
            ctrl_len = 7; ep0_send_chunk();
        } else if (bReq == 0x22U) {                     /* SET_CONTROL_LINE_STATE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else {
            ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL);
        }
    } else {
        ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL);
    }
}

static void ep0_in(void) {
    ep_clear_ctr_tx(0);
    if (ctrl_pending == CTRL_SET_ADDRESS) {
        USB->DADDR = usb_addr | 0x80U;
        ctrl_pending = CTRL_IDLE;
        ep_set_rx(0, EP_VALID);
    } else if (ctrl_pending == CTRL_DATA_IN) {
        if (ctrl_pos < ctrl_len) { ep0_send_chunk(); return; }
        ctrl_pending = CTRL_STATUS_OUT;
        ep_set_rx(0, EP_VALID);
    } else if (ctrl_pending == CTRL_STATUS_IN) {
        ctrl_pending = CTRL_IDLE;
        ep_set_rx(0, EP_VALID);
    }
}

static void ep0_out(void) {
    unsigned cnt = USB_BD_AT(0)->COUNT_RX & 0x3FFU;
    ep_clear_ctr_rx(0);
    if (ctrl_pending == CTRL_DATA_OUT) {
        pma_read16(PMA_EP0_RX, ctrl_buf + ctrl_pos, cnt);
        ctrl_pos += cnt;
        if (ctrl_pos >= ctrl_len) {
            unsigned i; for (i = 0; i < 7; i++) lc[i] = ctrl_buf[i];
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else {
            ep_set_rx(0, EP_VALID);
        }
    } else if (ctrl_pending == CTRL_STATUS_OUT) {
        ctrl_pending = CTRL_IDLE;
        ep_set_rx(0, EP_VALID);
    } else {
        ep_set_rx(0, EP_VALID);
    }
}

/* ---------- Bulk IN driver API (fixed flow control) ---------- */
int cdc_send_data(const unsigned char *data, unsigned len) {
    if (bulk_tx_busy || len == 0 || len > 64) return -1;
    { unsigned i; for (i = 0; i < len; i++) bulk_tx_buf[i] = data[i]; }
    bulk_tx_len  = len;
    bulk_tx_busy = 1;
    pma_write16(PMA_EP1_TX, bulk_tx_buf, len);
    USB_BD_AT(1)->COUNT_TX = (unsigned short)len;
    ep_set_tx(1, EP_VALID);
    return 0;
}

static void ep1_out(void) {
    unsigned char buf[64];
    unsigned cnt = USB_BD_AT(1)->COUNT_RX & 0x3FFU;
    ep_clear_ctr_rx(1);
    pma_read16(PMA_EP1_RX, buf, cnt);

    if (cnt > 0) {
        if (buf[0] == 1) {
            GPIOC->BRR = (1U << 13);   /* LED ON (pull low) */
        } else if (buf[0] == 0) {
            GPIOC->BSRR = (1U << 13);  /* LED OFF (pull high) */
            }
        /* echo back, but only if the IN path is free */
        if (!bulk_tx_busy && cnt > 0) {
            cdc_send_data(buf, cnt);
        }
    }
    ep_set_rx(1, EP_VALID);
}

/* ---------- ISR ---------- */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    unsigned istr = USB->ISTR;

    if (istr & ISTR_RESET) {
        USB->ISTR = ~ISTR_RESET;
        usb_addr = 0; ctrl_pending = CTRL_IDLE; bulk_tx_busy = 0;
        USB->DADDR = DADDR_EF;  /* EF, ADD[0-6] = 0 */
        USB->CNTR  = 0;
        ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | (EP_NAK << 4) | (EP_VALID << 12));
        USB_BD_AT(0)->COUNT_RX = ((0x1U << 15) | (0x1U << 10)); // 64 byte == BLSIZE=1 -> 32B units, NUM_BLOCK=1 -> 2 blocks
        USB_BD_AT(0)->COUNT_TX = 0;
        USB->CNTR = CNTR_CTRM | CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
        return;
    }
    if (istr & ISTR_WKUP) {
        USB->ISTR = ~ISTR_WKUP;
        return;
    }
    if (istr & ISTR_SUSP) {
        USB->ISTR = ~ISTR_SUSP;
        return;
    }
    if (istr & ISTR_CTR) {
        unsigned ep = (istr & ISTR_EP_ID);
        unsigned r  = USB->EPR[ep];
        if (ep == 0) {
            if      (r & EP_SETUP)  ep0_setup();
            else if (r & EP_CTR_TX) ep0_in();
            else if (r & EP_CTR_RX) ep0_out();
        } else if (ep == 1) {
            if      (r & EP_CTR_RX) ep1_out();
            else if (r & EP_CTR_TX) {
                ep_clear_ctr_tx(1);
                bulk_tx_busy = 0;               /* IN complete, safe to reuse */
            }
        } else {
            USB->EPR[ep] = EP_CTR_TX | EP_CTR_RX;
        }
    }
    /* Clear any other unhandled ISTR bits like ESOF (0x0100) */
    if (istr & (ISTR_SOF | ISTR_ESOF)) {
        USB->ISTR = ~(istr & (ISTR_SOF | ISTR_ESOF));
    }
}