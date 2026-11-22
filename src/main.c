#include <timer.h>
#include <rcc.h>
#include <systick.h>
#include <gpio.h>
#include <spi.h>
#include <i2c.h>
#include <gt911.h>
#include <nested-vector.h>
#include <sys-ctrl-blk.h>
#include <usb-core.h>
#include <stddef.h>


void crt0_init(void){
    extern volatile uint8_t  _sbss[], _ebss [], _sidata[];
    extern volatile uint8_t  _sdata[], _edata[];
    extern void (* const g_pfnVectors[])(void);

    volatile uint32_t *src, *dst;
    volatile uint8_t *byte_src, *byte_dst;

    SCB->VTOR = (uint32_t)g_pfnVectors;

    // Copy initialized data from Flash to RAM (optimize for large chunks)
    src = (volatile uint32_t *)&_sidata;
    dst = (volatile uint32_t *) &_sdata;
    while (dst + 1 <= (volatile uint32_t *) &_edata) {  *dst++ = *src++;  }
    // Handle remaining bytes (if the section size isn't a multiple of word size)
    byte_src = (volatile uint8_t *) src;
    byte_dst = (volatile uint8_t *) dst;
    while (byte_dst < (volatile uint8_t *) &_edata) { *byte_dst++ = *byte_src++;  }
    // Clear the .bss section (optimize for large chunks)
    dst = (volatile uint32_t *) &_sbss;
    while (dst + 1 <= (volatile uint32_t *) &_ebss) { *dst++ = 0; }
    // Handle remaining bytes in .bss
    byte_dst = (volatile uint8_t *) dst;
    while (byte_dst < (volatile uint8_t *) &_ebss) { *byte_dst++ = 0; }
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *p = (uint8_t *)dst;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return dst;
}

uint32_t irq_count = 0; // debug

extern uint8_t  bulk_tx_busy;

/* tiny unsigned->decimal-ASCII, no libc dependency */
static unsigned u16_to_dec(unsigned val, char *out)
{
    char tmp[6];
    unsigned n = 0;
    if (val == 0U) { out[0] = '0'; return 1U; }
    while (val > 0U && n < sizeof(tmp)) {
        tmp[n++] = (char)('0' + (val % 10U));
        val /= 10U;
    }
    for (unsigned i = 0; i < n; i++) out[i] = tmp[n - 1U - i];
    return n;
}

/* Builds "id,x,y;id,x,y;...\n" for up to GT911_MAX_POINTS points.
   Bails out early if it would overflow max_len (keeps us under 64B/packet). */
static unsigned format_multi_touch(char *line, unsigned max_len, const struct gt911_point_t *pts, unsigned n)
{
    unsigned p = 0;

    if (max_len == 0)
        return 0;

    for (unsigned i = 0; i < n; i++) {

        unsigned id = pts[i].id;
        unsigned x  = pts[i].x;
        unsigned y  = pts[i].y;

        /* Reserve space for newline */
        if (p + 14U >= max_len)
            break;

        p += u16_to_dec(id, line + p);
        line[p++] = ',';

        p += u16_to_dec(x, line + p);
        line[p++] = ',';

        p += u16_to_dec(y, line + p);
        line[p++] = ';';
    }

    line[p++] = '\n';

    return p;
}


int main(void)
{
    /* PC13 LED */
    RCC->APB2ENR |= (1U << 4);

    GPIOC->CRH = (GPIOC->CRH & ~0x00F00000U) | 0x00200000U;
    GPIOC->BSRR = (1U << 13);

    /* USB clock: 72 MHz SYSCLK, 48 MHz USB */
    usb_clock_init();

    /* I2C1 */
    i2c_init(I2C1, FREQ_400KHZ);

    /* GT911 initialization */
    if (gt911_init(I2C1) != 0) {
        while (1);
    }

    /* Enable interrupts */
    __asm__ volatile ("cpsie i");

    /* USB device initialization */
    usb_init();

    struct gt911_point_t points[GT911_MAX_TOUCHES] = {0};
    uint8_t count = 0;
    char line[64];
    unsigned len;



    while (1)
    {
        if (gt911_irq_flag)
        {
            gt911_irq_flag = 0;
            if (gt911_read_points(points, &count) == 0)
            {
                if (count > 0)
                {
                    len = format_multi_touch(line, sizeof(line), points, count);
                    if (!bulk_tx_busy && len > 0 && len <= 64)
                    {
                        cdc_send_data((const uint8_t *)line, len);
                    }
                }
            }
        }

        /* Wait for touch interrupt or USB interrupt */
        __asm__ volatile ("wfi");
    }

    return 0;
}