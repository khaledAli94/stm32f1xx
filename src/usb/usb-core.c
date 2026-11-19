#include <usb-core.h>

/*
 * CDC requires 2 interfaces, 3 endpoints, and complex union/call-management descriptors
 * per modemax bulk packet sz
 * Low-Speed  1.5 Mbit/s 8 bytes
 * Full-Speed 12 Mbit/s 64 bytes
 * High-Speed 480 Mbit/s 512 bytes
*/


static unsigned char    usb_addr;
static enum usb_ctrl_state_t  ctrl_pending;
static unsigned char    ctrl_buf[128];
static unsigned         ctrl_len;
static unsigned         ctrl_pos;

// static unsigned char  bulk_tx_busy;
unsigned char  bulk_tx_busy;
static unsigned char  bulk_tx_buf[64];
static unsigned       bulk_tx_len;


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
static inline void ep_set(unsigned ep, unsigned mask, unsigned data)
{
    unsigned old = USB->EPR[ep];
    unsigned rc_w0 = EP_CTR_RX | EP_CTR_TX;
    unsigned toggle = EP_DTOG_RX | EP_STAT_RX_MASK | EP_DTOG_TX | EP_STAT_TX_MASK;
    unsigned rw = EP_TYPE_MASK | EP_KIND | EP_ADDR_MASK;

    unsigned wr0 = rc_w0 & (~mask | data);
    unsigned wr1 = (mask & toggle) & (old ^ data);
    unsigned wr2 = rw & ((old & ~mask) | data);

    USB->EPR[ep] = (unsigned short)(wr0 | wr1 | wr2);
}

static void ep_set_tx(unsigned ep, unsigned stat)
{
    ep_set(ep, EP_STAT_TX_MASK, EP_TX_STAT(stat));
}
static void ep_set_rx(unsigned ep, unsigned stat)
{
    ep_set(ep, EP_STAT_RX_MASK, EP_RX_STAT(stat));
}
static void ep_clear_ctr_rx(unsigned ep)
{
    ep_set(ep, EP_CTR_RX, 0);
}
static void ep_clear_ctr_tx(unsigned ep)
{
    ep_set(ep, EP_CTR_TX, 0);
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
    ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_VALID));
    USB_BD_AT(0)->ADDR_RX = PMA_EP0_RX; USB_BD_AT(0)->COUNT_RX = COUNT_RX_64B;
    USB_BD_AT(0)->ADDR_TX = PMA_EP0_TX; USB_BD_AT(0)->COUNT_TX = 0;

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

    if ((bmReq & 0x60U) == USB_REQ_STANDARD) {
        if (bReq == REQ_GET_DESCRIPTOR) {
            unsigned type = wVal >> 8;
            const unsigned char *src = 0; unsigned n = 0;
            if      (type == USB_DESC_DEVICE) { src = devd; n = sizeof(devd); }
            else if (type == USB_DESC_CONFIGURATION) { src = cfgd; n = sizeof(cfgd); }
            else if (type == USB_DESC_STRING && (wVal & 0xFFU) == 0) { src = str0; n = sizeof(str0);}
            if (!src) { ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL); return; }
            if (n > sizeof(ctrl_buf)) n = sizeof(ctrl_buf);
            if (n > wLen) n = wLen;
            { unsigned i; for (i = 0; i < n; i++) ctrl_buf[i] = src[i]; }
            ctrl_len = n;
            ep0_send_chunk();
        } else if (bReq == REQ_SET_ADDRESS) {
            usb_addr = (unsigned char)(wVal & 0x7FU);
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_SET_ADDRESS;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_SET_CONFIGURATION) {
            if (wVal) {
                USB_BD_AT(1)->ADDR_TX = PMA_EP1_TX;  USB_BD_AT(1)->COUNT_TX = 0;
                USB_BD_AT(1)->ADDR_RX = PMA_EP1_RX;  USB_BD_AT(1)->COUNT_RX = COUNT_RX_64B;
                ep_set(1, 0xFFFFU, EP_ADDR(1) | EP_TYPE_BULK | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_VALID));
                USB_BD_AT(2)->ADDR_TX = PMA_EP2_TX;  USB_BD_AT(2)->COUNT_TX = 0;
                ep_set(2, 0xFFFFU, EP_ADDR(2) | EP_TYPE_INT | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_DISABLED));
            }
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_GET_STATUS) {
            ctrl_buf[0] = 0; ctrl_buf[1] = 0;
            ctrl_len = 2;
            ep0_send_chunk();
        } else if (bReq == REQ_CLEAR_FEATURE || bReq == REQ_SET_FEATURE) {
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_SET_INTERFACE) {
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_GET_INTERFACE) {
            ctrl_buf[0] = 0;
            ctrl_len = 1;
            ep0_send_chunk();
        } else {
            ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL);
        }
    } else if ((bmReq & 0x60U) == USB_REQ_CLASS) {
        if (bReq == CDC_SET_LINE_CODING) {
            ctrl_len = (wLen < 7) ? wLen : 7;
            ctrl_pos = 0;
            ctrl_pending = CTRL_DATA_OUT;
            ep_set_rx(0, EP_VALID);
        } else if (bReq == CDC_GET_LINE_CODING) {
            unsigned i; for (i = 0; i < 7; i++) ctrl_buf[i] = lc[i];
            ctrl_len = 7; ep0_send_chunk();
        } else if (bReq == CDC_SET_CONTROL_LINE_STATE) {
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
        USB->DADDR = usb_addr | DADDR_EF;
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
    unsigned cnt = USB_BD_AT(0)->COUNT_RX & COUNT_NBYTES_MASK;
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
    unsigned cnt = USB_BD_AT(1)->COUNT_RX & COUNT_NBYTES_MASK;
    ep_clear_ctr_rx(1);
    pma_read16(PMA_EP1_RX, buf, cnt);

    if (cnt > 0) {
        if (buf[0] == 1) {
            GPIOC->BRR = (1U << 13);   /* LED ON (pull low) */
        } else if (buf[0] == 0) {
            GPIOC->BSRR = (1U << 13);  /* LED OFF (pull high) */
            }
        /* echo back, but only if the IN path is free */
        // if (!bulk_tx_busy && cnt > 0) {
        //     cdc_send_data(buf, cnt);
        // }
    }
    ep_set_rx(1, EP_VALID);
}

/* ---------- ISR ---------- */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    unsigned istr = USB->ISTR;

    if (istr & ISTR_RESET) {
        USB->ISTR = ~ISTR_RESET;
        usb_addr = 0;
        ctrl_pending = CTRL_IDLE;
        bulk_tx_busy = 0;
        USB->DADDR = DADDR_EF;  /* EF, ADD[0-6] = 0 */
        USB->CNTR  = 0;
        ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_VALID));
        USB_BD_AT(0)->ADDR_RX = PMA_EP0_RX;
        USB_BD_AT(0)->COUNT_RX = COUNT_RX_64B;  // 64 byte == BLSIZE=1 -> 32B units, NUM_BLOCK=1 -> 2 blocks
        USB_BD_AT(0)->ADDR_TX = PMA_EP0_TX;
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

/* usgae */
// int main(){

//     /* Enable GPIOC clock */
//     RCC->APB2ENR |= (1U << 4);  /* IOPCEN */

//     /* Configure PC13 as Output Push-Pull 2MHz */
//     GPIOC->CRH = (GPIOC->CRH & ~0x00F00000U) | 0x00200000U;

//     /* Default LED state: OFF (active low -> set high) */
//     GPIOC->BSRR = (1U << 13);

//     usb_clock_init();
//     __asm__("cpsie i");  /* Enable global interrupts */
//     usb_init();

    
//     while (1)
//     {
//         __asm__("wfi");  /* wakeup on intterupt */
//     }
    

//     return 0;
// }