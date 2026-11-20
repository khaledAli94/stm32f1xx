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

int main()
{
    /* 1. Initialize system clocks (72 MHz PLL or default 8 MHz HSI) */
    rcc_sysclk_hse_72mhz();

    systick_init();
    tim2_init();
    __asm__ volatile ("cpsie i");
    
    /* 2. Wait for GT911 internal power-on reset (~100 ms since RST is tied to 3.3V) */
    delay_ms(100);

    /* 3. Initialize I2C1 at 400 kHz Fast Mode (PB6 = SCL, PB7 = SDA) */
    i2c_init(I2C1, 400000U);

    /* 4. Probe GT911 at 0x5D, configure PB5 as floating input, clear status */
    if (gt911_init(I2C1) != 0) {
        while (1) {
            /* Error: GT911 did not respond at 0x5D (check pull-ups or if latched at 0x14) */
        }
    }

    struct gt911_point_t points[GT911_MAX_TOUCHES] = {0};
    uint8_t count = 0;


    while (1) {
        if (gt911_irq_flag) {
            gt911_irq_flag = 0;

            /* Reads 0x814E + coordinates and writes 0x00 to 0x814E so INT stops pulsing */
            if (gt911_read_points(points, &count) == 0 && count > 0) {
                for (uint8_t i = 0; i < count; i++) {
                    uint8_t  id   = points[i].id;
                    uint16_t x    = points[i].x;
                    uint16_t y    = points[i].y;
                    uint16_t size = points[i].size;

                    /* Process touch coordinates (x, y) here */
                    (void)id; (void)x; (void)y; (void)size;
                }
            }
        } else {
            /* Sleep CPU until EXTI5 (touch) or SysTick (1 ms) fires */
            __asm__ volatile ("wfi");
        }
    }

    return 0;
}