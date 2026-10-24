#include <clock.h>
#include <usb-regs.h>
#include <gpio.h>
#include <nested-vector.h>
#include <flash-regs.h>

/* ---------- state (plain globals, no struct if you prefer) ---------- */
static unsigned char  usb_addr;
static unsigned       ctrl_pending;      /* 0 idle, 1 OUT-data, 2 set-addr,
                                            3 IN-chunks, 5 IN-ZLP, 6 OUT-ZLP */
static unsigned char  ctrl_buf[128];
static unsigned       ctrl_len;
static unsigned       ctrl_pos;

static unsigned char  bulk_tx_busy;
static unsigned char  bulk_tx_buf[64];
static unsigned       bulk_tx_len;

static unsigned char  lc[7] = { 0x00, 0xC2, 0x01, 0x00, 0x00, 0x00, 0x08 };

/* ---------- descriptors ---------- */
static const unsigned char devd[] = {
    18, 0x01, 0x00, 0x02, 0x02, 0x00, 0x00, 0x40,
    0x83, 0x04, 0x40, 0x57, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x01
};
static const unsigned char cfgd[] = {
    9, 0x02, 0x43, 0x00, 0x02, 0x01, 0x00, 0x80, 0x32,
    9, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00,
    5, 0x24, 0x00, 0x10, 0x01,
    5, 0x24, 0x01, 0x00, 0x01,
    4, 0x24, 0x02, 0x02,
    5, 0x24, 0x06, 0x00, 0x01,
    7, 0x05, 0x82, 0x03, 0x08, 0x00, 0x0A,
    9, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
    7, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
    7, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00
};
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

/* ---------- EPnR robust register manipulation helper (adapted from f1usb) ---------- */
static void ep_set(unsigned ep, unsigned mask, unsigned data) {
    unsigned old = USB->EPR[ep];
    unsigned rc_w0  = 0x8080U; /* CTR_RX (0x8000) | CTR_TX (0x0080) */
    unsigned toggle = 0x7030U; /* DTOG_RX (0x4000) | STAT_RX (0x3000) | DTOG_TX (0x0040) | STAT_TX (0x0030) */
    unsigned rw     = 0x070FU; /* EP_TYPE (0x0600) | EP_KIND (0x0100) | EA (0x000F) */

    unsigned wr0 = rc_w0 & (~mask | data);
    unsigned wr1 = (mask & toggle) & (old ^ data);
    unsigned wr2 = rw & ((old & ~mask) | data);

    USB->EPR[ep] = (unsigned short)(wr0 | wr1 | wr2);
}

static void ep_set_tx(unsigned ep, unsigned s) {
    ep_set(ep, 0x0030U, s << 4);
}
static void ep_set_rx(unsigned ep, unsigned s) {
    if (ep == 0) USB_BD_AT(0)->COUNT_RX = 0x8800;
    else if (ep == 1) USB_BD_AT(1)->COUNT_RX = 0x8800;
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

    RCC->APB2ENR |= (1U << 2);                   /* GPIOAEN */
    RCC->APB1ENR |= (1U << 23);                  /* USBEN  */

    /* PA11 / PA12 as input floating */
    GPIOA->CRH = (GPIOA->CRH & ~0x000FF000U) | 0x00044000U;

    for (d = 0x10000; d; --d) { }                /* settle */

    USB->CNTR = CNTR_FRES;
    for (d = 0x10000; d; --d) { }
    USB->CNTR = 0;
    USB->ISTR = 0;
    USB->BTABLE = 0;                             /* BTABLE at PMA byte 0 */

    /* EP0: control, RX valid, TX NAK */
    ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | (EP_NAK << 4) | (EP_VALID << 12));
    USB_BD_AT(0)->ADDR_TX  = PMA_EP0_TX;  USB_BD_AT(0)->COUNT_TX = 0;
    USB_BD_AT(0)->ADDR_RX  = PMA_EP0_RX;  USB_BD_AT(0)->COUNT_RX = 0x8800;

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
    ctrl_pending = 3;
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

    ctrl_len = 0; ctrl_pos = 0; ctrl_pending = 0;

    if ((bmReq & 0x60U) == 0x00U) {                    /* standard */
        if (bReq == 0x06U) {                            /* GET_DESCRIPTOR */
            unsigned type = wVal >> 8;
            const unsigned char *src = 0; unsigned n = 0;
            if      (type == 0x01U) { src = devd; n = sizeof(devd); }
            else if (type == 0x02U) { src = cfgd; n = sizeof(cfgd); }
            else if (type == 0x03U && (wVal & 0xFFU) == 0) {
                src = str0; n = sizeof(str0);
            }
            if (!src) { ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL); return; }
            if (n > sizeof(ctrl_buf)) n = sizeof(ctrl_buf);
            if (n > wLen) n = wLen;
            { unsigned i; for (i = 0; i < n; i++) ctrl_buf[i] = src[i]; }
            ctrl_len = n;
            ep0_send_chunk();
        } else if (bReq == 0x05U) {                     /* SET_ADDRESS */
            usb_addr = (unsigned char)(wVal & 0x7FU);
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 2;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x09U) {                     /* SET_CONFIGURATION */
            if (wVal) {
                USB_BD_AT(1)->ADDR_TX = PMA_EP1_TX;  USB_BD_AT(1)->COUNT_TX = 0;
                USB_BD_AT(1)->ADDR_RX = PMA_EP1_RX;  USB_BD_AT(1)->COUNT_RX = 0x8800;
                ep_set(1, 0xFFFFU, 0x0001U | EP_TYPE_BULK
                                 | (EP_NAK << 4) | (EP_VALID << 12));

                USB_BD_AT(2)->ADDR_TX = PMA_EP2_TX;  USB_BD_AT(2)->COUNT_TX = 0;
                ep_set(2, 0xFFFFU, 0x0002U | EP_TYPE_INT
                                 | (EP_NAK << 4) | (EP_DISABLED << 12));
            }
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 5;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x00U) {                     /* GET_STATUS */
            ctrl_buf[0] = 0; ctrl_buf[1] = 0;
            ctrl_len = 2;
            ep0_send_chunk();
        } else if (bReq == 0x01U || bReq == 0x03U) {    /* CLEAR_FEATURE / SET_FEATURE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 5;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == 0x0BU) {                     /* SET_INTERFACE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 5;
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
            ctrl_pending = 1;
            ep_set_rx(0, EP_VALID);
        } else if (bReq == 0x21U) {                     /* GET_LINE_CODING */
            unsigned i; for (i = 0; i < 7; i++) ctrl_buf[i] = lc[i];
            ctrl_len = 7; ep0_send_chunk();
        } else if (bReq == 0x22U) {                     /* SET_CONTROL_LINE_STATE */
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 5;
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
    if (ctrl_pending == 2) {
        USB->DADDR = usb_addr | 0x80U;
        ctrl_pending = 0;
        ep_set_rx(0, EP_VALID);
    } else if (ctrl_pending == 3) {
        if (ctrl_pos < ctrl_len) { ep0_send_chunk(); return; }
        ctrl_pending = 6;
        ep_set_rx(0, EP_VALID);
    } else if (ctrl_pending == 5) {
        ctrl_pending = 0;
        ep_set_rx(0, EP_VALID);
    }
}

static void ep0_out(void) {
    unsigned cnt = USB_BD_AT(0)->COUNT_RX & 0x3FFU;
    ep_clear_ctr_rx(0);
    if (ctrl_pending == 1) {
        pma_read16(PMA_EP0_RX, ctrl_buf + ctrl_pos, cnt);
        ctrl_pos += cnt;
        if (ctrl_pos >= ctrl_len) {
            unsigned i; for (i = 0; i < 7; i++) lc[i] = ctrl_buf[i];
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = 5;
            ep_set_tx(0, EP_VALID);
        } else {
            ep_set_rx(0, EP_VALID);
        }
    } else if (ctrl_pending == 6) {
        ctrl_pending = 0;
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
    /* echo, but only if the IN path is free */
    if (!bulk_tx_busy && cnt > 0) {
        cdc_send_data(buf, cnt);
    }
    ep_set_rx(1, EP_VALID);
}

/* ---------- ISR ---------- */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    unsigned istr = USB->ISTR;

    if (istr & ISTR_RESET) {
        USB->ISTR = ~ISTR_RESET;
        usb_addr = 0; ctrl_pending = 0;
        bulk_tx_busy = 0;
        USB->DADDR = 0;
        USB->CNTR  = 0;
        ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | (EP_NAK << 4) | (EP_VALID << 12));
        USB_BD_AT(0)->COUNT_TX = 0;
        USB_BD_AT(0)->COUNT_RX = 0x8800;
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
        unsigned ep = istr & ISTR_EP_ID;
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
    if (istr & 0x0300U) {
        USB->ISTR = ~(istr & 0x0300U);
    }
}