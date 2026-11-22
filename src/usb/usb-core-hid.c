#include <usb-core.h>

/*
 * CDC requires 2 interfaces, 3 endpoints, and complex union/call-management descriptors
 * HID requires 1 interface, 1–2 interrupt-endpoints, HID-Report-descriptors
 * per modemax bulk packet sz
 * Low-Speed  1.5 Mbit/s 8 bytes
 * Full-Speed 12 Mbit/s 64 bytes
 * High-Speed 480 Mbit/s 512 bytes
*/


static uint8_t    usb_addr;
static enum usb_ctrl_state_t  ctrl_pending;
static uint8_t    ctrl_buf[384]; /* internal buffer used for control transfers */
static uint32_t   ctrl_len;
static uint32_t   ctrl_pos;

// static uint8_t  bulk_tx_busy;
uint8_t  bulk_tx_busy;
static uint8_t  bulk_tx_buf[64];
static uint32_t bulk_tx_len;

static uint8_t  usb_configured;
static uint8_t  hid_idle_rate;

static void hid_make_empty_report(void)
{
    uint32_t i;

    for (i = 0; i < HID_MOUSE_REPORT_SIZE; i++) {
        ctrl_buf[i] = 0;
    }

    ctrl_buf[0] = HID_MOUSE_REPORT_ID;
    ctrl_len = HID_MOUSE_REPORT_SIZE;
}

void usb_clock_init(void) {
    /* Flash: 2 wait states + prefetch (needed for 72 MHz on F103) */
    flash_set_latency(FREQ_72MHZ);

    RCC->CR |= (1U << 16);                       /* HSEON  */
    while (!(RCC->CR & (1U << 17))) { }           /* HSERDY */

    RCC->CFGR = ((uint32_t)CLOCK_PLL   << 0)
              | ((uint32_t)AHB_DIV_1   << 4)
              | ((uint32_t)APB_DIV_2   << 8)
              | ((uint32_t)APB_DIV_1   << 11)
              | (1U << 16)                       /* PLLSRC = HSE */
              | ((uint32_t)PLL_MUL_9   << 18)
              | ((uint32_t)USB_DIV_1_5 << 22);

    RCC->CR |= (1U << 24);                       /* PLLON  */
    while (!(RCC->CR & (1U << 25))) { }           /* PLLRDY */

    RCC->CFGR = (RCC->CFGR & ~0x3U) | CLOCK_PLL; /* switch SYSCLK */
    while (((RCC->CFGR >> 2) & 0x3U) != (uint32_t)CLOCK_PLL) { }
}

/* ---------- PMA helpers (16-bit only, 2:1 hole) ---------- */
static void pma_write16(uint32_t b, const uint8_t *s, uint32_t n) {
    uint32_t i;
    for (i = 0; i < n; i += 2) {
        uint16_t w = s[i];
        if (i + 1 < n) w |= (uint16_t)s[i + 1] << 8;
        PMA_BYTE(b + i) = w;
    }
}
static void pma_read16(uint32_t b, uint8_t *d, uint32_t n) {
    uint32_t i;
    for (i = 0; i < n; i += 2) {
        uint16_t w = PMA_BYTE(b + i);
        d[i] = (uint8_t)(w & 0xFF);
        if (i + 1 < n) d[i + 1] = (uint8_t)(w >> 8);
    }
}

/* ---------- EPnR and packet transfer events handlers --------- */
static inline void ep_set(uint32_t ep, uint32_t mask, uint32_t data)
{
    uint32_t old = USB->EPR[ep];
    uint32_t rc_w0 = EP_CTR_RX | EP_CTR_TX;
    uint32_t toggle = EP_DTOG_RX | EP_STAT_RX_MASK | EP_DTOG_TX | EP_STAT_TX_MASK;
    uint32_t rw = EP_TYPE_MASK | EP_KIND | EP_ADDR_MASK;

    uint32_t wr0 = rc_w0 & (~mask | data);
    uint32_t wr1 = (mask & toggle) & (old ^ data);
    uint32_t wr2 = rw & ((old & ~mask) | data);

    USB->EPR[ep] = (uint16_t)(wr0 | wr1 | wr2);
}

static void ep_set_tx(uint32_t ep, uint32_t stat)
{
    ep_set(ep, EP_STAT_TX_MASK, EP_TX_STAT(stat));
}
static void ep_set_rx(uint32_t ep, uint32_t stat)
{
    ep_set(ep, EP_STAT_RX_MASK, EP_RX_STAT(stat));
}
static void ep_clear_ctr_rx(uint32_t ep)
{
    ep_set(ep, EP_CTR_RX, 0);
}
static void ep_clear_ctr_tx(uint32_t ep)
{
    ep_set(ep, EP_CTR_TX, 0);
}

/* ---------- init ---------- */
void usb_init(void) {
    RCC->APB2ENR |= (1U << 2);  /* GPIOA-EN */
    RCC->APB1ENR |= (1U << 23); /* USB-EN  */

    /* PA11 / PA12 as input floating */
    GPIOA->CRH = (GPIOA->CRH & ~0x000FF000U) | 0x00044000U;
    delay_ms(10);

    USB->CNTR = CNTR_FRES;
    delay_ms(10);

    USB->CNTR = 0;
    USB->ISTR = 0;
    USB->BTABLE = 0; /* BTABLE at PMA byte 0 */

    /* EP0: control, RX valid, TX NAK */
    ep_set(0, 0xFFFFU, EP_TYPE_CONTROL | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_VALID));
    USB_BD_AT(0)->ADDR_RX = PMA_EP0_RX; USB_BD_AT(0)->COUNT_RX = COUNT_RX_64B;
    USB_BD_AT(0)->ADDR_TX = PMA_EP0_TX; USB_BD_AT(0)->COUNT_TX = 0;

    USB->CNTR  = CNTR_CTRM | CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
    NVIC->ISER[0] = (1U << 20);   /* NVIC USB_LP */
}

/* ---------- control-transfer helpers ---------- */
static void ep0_send_chunk(void) {
    uint32_t n = ctrl_len - ctrl_pos;
    if (n > 64) n = 64;
    pma_write16(PMA_EP0_TX, ctrl_buf + ctrl_pos, n);
    USB_BD_AT(0)->COUNT_TX = (uint16_t)n;
    ctrl_pos += n;
    ctrl_pending = CTRL_DATA_IN;
    ep_set_tx(0, EP_VALID);
}

static void ep0_setup(void) {
    uint8_t s[8];
    uint32_t bmReq, bReq, wVal, wLen;

    pma_read16(PMA_EP0_RX, s, 8);
    ep_clear_ctr_rx(0);
    bmReq = s[0];
    bReq = s[1];
    wVal  = s[2] | ((uint32_t)s[3] << 8);
    wLen  = s[6] | ((uint32_t)s[7] << 8);

    ctrl_len = 0; ctrl_pos = 0; ctrl_pending = CTRL_IDLE;

    if ((bmReq & 0x60U) == USB_REQ_STANDARD) {
        if (bReq == REQ_GET_DESCRIPTOR) {
            uint32_t type = wVal >> 8;
            const uint8_t *src = 0; uint32_t n = 0;
            if      (type == USB_DESC_DEVICE) { src = devd; n = sizeof(devd); }
            else if (type == USB_DESC_CONFIGURATION) { src = cfgd; n = sizeof(cfgd); }
            else if (type == USB_DESC_STRING && (wVal & 0xFFU) == 0) { src = str0; n = sizeof(str0);}
            else if (type == USB_DESC_HID) { src = cfgd + 18U; n = 9U; } // HID
            else if (type == USB_DESC_REPORT) { src = hid_report_desc; n = sizeof(hid_report_desc); } // HID

            if (!src) { ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL); return; }
            if (n > sizeof(ctrl_buf)) n = sizeof(ctrl_buf);
            if (n > wLen) n = wLen;
            { uint32_t i; for (i = 0; i < n; i++) ctrl_buf[i] = src[i]; }
            ctrl_len = n;
            ep0_send_chunk();
        } else if (bReq == REQ_SET_ADDRESS) {
            usb_addr = (uint8_t)(wVal & 0x7FU);
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_SET_ADDRESS;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_SET_CONFIGURATION) {
            /* HID usees EP1 as INT-EP, whereas CDC uses EP1 as BULK IN/OUT , EP2 as ser-notif-EP */
            if (wVal == 1U) {
                USB_BD_AT(1)->ADDR_TX = PMA_EP1_TX;  USB_BD_AT(1)->COUNT_TX = 0;
                ep_set(1, 0xFFFFU, EP_ADDR(1) | EP_TYPE_INT | EP_TX_STAT(EP_NAK) | EP_RX_STAT(EP_DISABLED));
                bulk_tx_busy = 0;
                usb_configured = 1;
            } else if (wVal == 0U) {
                bulk_tx_busy = 0;
                usb_configured = 0;
                ep_set(1, 0xFFFFU, EP_ADDR(1) | EP_TYPE_INT | EP_TX_STAT(EP_DISABLED) | EP_RX_STAT(EP_DISABLED));
            } else {
                ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL); return;
            }
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == REQ_GET_CONFIGURATION) { // HID and CDC
            ctrl_buf[0] = usb_configured ? 1U : 0U;
            ctrl_len = 1;
            ep0_send_chunk();
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
        if (bReq == HID_SET_IDLE) {
            hid_idle_rate = (uint8_t)(wVal >> 8);
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == HID_GET_IDLE) {
            ctrl_buf[0] = hid_idle_rate;
            ctrl_len = 1;
            ep0_send_chunk();
        } else if (bReq == HID_SET_PROTOCOL) {
            USB_BD_AT(0)->COUNT_TX = 0;
            ctrl_pending = CTRL_STATUS_IN;
            ep_set_tx(0, EP_VALID);
        } else if (bReq == HID_GET_PROTOCOL) {
            ctrl_buf[0] = 1; /* Report protocol */
            ctrl_len = 1;
            ep0_send_chunk();
        } else if (bReq == HID_GET_REPORT) {
            hid_make_empty_report();
            if (ctrl_len > wLen) ctrl_len = wLen;
            ep0_send_chunk();
        } else {
            ep_set_tx(0, EP_STALL); ep_set_rx(0, EP_STALL);
        }
    }    
    else {
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
    uint32_t cnt = USB_BD_AT(0)->COUNT_RX & COUNT_NBYTES_MASK;
    ep_clear_ctr_rx(0);
    if (ctrl_pending == CTRL_DATA_OUT) {
        pma_read16(PMA_EP0_RX, ctrl_buf + ctrl_pos, cnt);
        ctrl_pos += cnt;
        if (ctrl_pos >= ctrl_len) {
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

/* ---------- HID INT IN API ---------- */
int hid_send_report(const uint8_t *data, uint32_t len) {
    if (!usb_configured || bulk_tx_busy || len == 0 || len > 64) return -1;
    { uint32_t i; for (i = 0; i < len; i++) bulk_tx_buf[i] = data[i]; }
    bulk_tx_len  = len;
    bulk_tx_busy = 1;

    pma_write16(PMA_EP1_TX, bulk_tx_buf, len);
    USB_BD_AT(1)->COUNT_TX = (uint16_t)len;
    /* CDC set EP1 to VALID for a bulk packet, where HID sets EP1 to VALID for interrupt-input-report */
    ep_set_tx(1, EP_VALID);
    return 0;
}

/* ---------- ISR ---------- */
void USB_LP_CAN1_RX0_IRQHandler(void) {
    uint32_t istr = USB->ISTR;

    if (istr & ISTR_RESET) {
        USB->ISTR = ~ISTR_RESET;
        usb_addr = 0;
        ctrl_pending = CTRL_IDLE;
        bulk_tx_busy = 0; usb_configured = 0; hid_idle_rate = 0;
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
        uint32_t ep = (istr & ISTR_EP_ID);
        uint32_t r  = USB->EPR[ep];
        if (ep == 0) {
            if      (r & EP_SETUP)  ep0_setup();
            else if (r & EP_CTR_TX) ep0_in();
            else if (r & EP_CTR_RX) ep0_out();
        } else if (ep == 1) { /* HID EP1 is IN-only,  CDC had ep1_out() for serial data received from the host,  HID touch normally only sends reports to the host.*/
            if (r & EP_CTR_TX) {
                ep_clear_ctr_tx(1);
                bulk_tx_busy = 0;               /* IN complete, safe to reuse */
            }
            if (r & EP_CTR_RX) {
                ep_clear_ctr_rx(1);
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