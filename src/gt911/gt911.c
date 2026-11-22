#include <gt911.h>
#include <gpio.h>
#include <rcc.h>
#include <systick.h>
#include <alter-func.h>
#include <external-intr.h>
#include <nested-vector.h>

/* 
 GT911 requires 16-bit big-endian reg addr, we pack that addr in raw i2c read/write 
 rst sequence : pulling (PB8) low for 15 ms while INT (PB5) low latches 0x5D addr
 INT pin must be floating input after reset sequence and trigger interrupt based on gt911 0x804D reg.
  */

volatile uint8_t gt911_irq_flag = 0;

/* gt911 expects two's complement additive checksum (~sum) + 1U <=> (0U - sum) */
static uint8_t calc_checksum(const uint8_t *cfg, uint16_t len)
{
    uint8_t sum = 0U;
    for (uint16_t i = 0U; i < len; i++) {
        sum = (uint8_t)(sum + cfg[i]);
    }
    return (uint8_t)(0U - sum);
}

static int gt911_read_reg(volatile struct i2c_t *i2c, uint16_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t addr_bytes[2] = { (uint8_t)(reg >> 8), (uint8_t)(reg & 0xFFU) };

    if (i2c_write(i2c, GT911_ADDR_5D, addr_bytes, 2U) != I2C_OK) return -1;
    return i2c_read(i2c, GT911_ADDR_5D, buf, len);
}

static int gt911_write_reg(volatile struct i2c_t *i2c, uint16_t reg, const uint8_t *data, uint16_t len)
{
    uint8_t tx[256];
    if (len > sizeof(tx) - 2U) return -1;

    tx[0] = (uint8_t)(reg >> 8);
    tx[1] = (uint8_t)(reg & 0xFFU);
    for (uint16_t i = 0; i < len; i++) tx[2U + i] = data[i];

    return i2c_write(i2c, GT911_ADDR_5D, tx, (uint16_t)(len + 2U));
}

int gt911_init(volatile struct i2c_t *i2c)
{
    rcc_gpiob_enable();
    rcc_afio_enable();

    /* 1. Reset sequence: PB8 (RST) + PB5 (INT) LOW -> RST HIGH -> INT floating */
    gpio_pin_mode(GPIOB, PIN_8, GPIO_OUTPUT_PP_2MHZ);
    gpio_pin_mode(GPIOB, PIN_5, GPIO_OUTPUT_PP_2MHZ);
    GPIOB->BRR = (1U << 8) | (1U << 5);
    delay_ms(15);

    GPIOB->BSRR = (1U << 8);
    delay_ms(6);

    gpio_pin_mode(GPIOB, PIN_5, GPIO_INPUT_FLOATING);
    delay_ms(60);

    /* 2. Verify Product ID ("911"), read INT polarity (0x804D), clear status */
    uint8_t id[4] = {0}, mod_sw1 = 0, zero = 0;
    if (gt911_read_reg(i2c, GT911_REG_PRODUCT_ID, id, 4U) != I2C_OK) return -1;
    if (id[0] != '9' || id[1] != '1' || id[2] != '1') return -1;
    if (gt911_read_reg(i2c, 0x804DU, &mod_sw1, 1U) != I2C_OK) return -1;
    if (gt911_write_reg(i2c, GT911_REG_POINT_STATUS, &zero, 1U) != I2C_OK) return -1;

    /* 3. Route PB5 to EXTI5 matching panel INT edge (0 = rising, 1 = falling) */
    AFIO->EXTICR2 = (AFIO->EXTICR2 & ~(0xFU << 4)) | (0x1U << 4);
    if ((mod_sw1 & 0x03U) == 1U) {
        EXTI->FTSR |=  (1U << 5);
        EXTI->RTSR &= ~(1U << 5);
    } else {
        EXTI->RTSR |=  (1U << 5);
        EXTI->FTSR &= ~(1U << 5);
    }

    EXTI->PR   = (1U << 5);
    EXTI->IMR |= (1U << 5);
    gt911_irq_flag = 0;
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    return 0;
}

int gt911_read_points(struct gt911_point_t *points, uint8_t *count)
{
    EXTI->IMR &= ~(1U << 5); // Mute PB6 (SCL) crosstalk during I2C transfer (mask)

    uint8_t status = 0, zero = 0;
    *count = 0;

    if (gt911_read_reg(I2C1, GT911_REG_POINT_STATUS, &status, 1U) == I2C_OK && (status & 0x80U)) {
        uint8_t n = status & 0x0FU;
        if (n > GT911_MAX_TOUCHES) n = GT911_MAX_TOUCHES;

        if (n > 0U) {
            uint8_t raw[GT911_MAX_TOUCHES * 8U];
            if (gt911_read_reg(I2C1, GT911_REG_POINT_DATA, raw, (uint16_t)(n * 8U)) == I2C_OK) {
                for (uint8_t i = 0; i < n; i++) {
                    uint8_t *p = &raw[i * 8U];
                    points[i].id   = p[0];
                    points[i].x    = (uint16_t)p[1] | ((uint16_t)p[2] << 8);
                    points[i].y    = (uint16_t)p[3] | ((uint16_t)p[4] << 8);
                    points[i].size = (uint16_t)p[5] | ((uint16_t)p[6] << 8);
                }
                *count = n;
            }
        }
        gt911_write_reg(I2C1, GT911_REG_POINT_STATUS, &zero, 1U);
    }

    EXTI->PR   = (1U << 5);
    EXTI->IMR |= (1U << 5);
    return 0;
}


/* test benchmark */

// int main()
// {
//     /* 1. Initialize system clocks (72 MHz PLL or default 8 MHz HSI) */
//     rcc_sysclk_hse_72mhz();

//     systick_init();
//     tim2_init();
//     __asm__ volatile ("cpsie i");
    
//     /* 2. Wait for GT911 internal power-on reset (~100 ms since RST is tied to 3.3V) */
//     delay_ms(100);

//     /* 3. Initialize I2C1 at 400 kHz Fast Mode (PB6 = SCL, PB7 = SDA) */
//     i2c_init(I2C1, 400000U);

//     /* 4. Probe GT911 at 0x5D, configure PB5 as floating input, clear status */
//     if (gt911_init(I2C1) != 0) {
//         while (1) {
//             /* Error: GT911 did not respond at 0x5D (check pull-ups or if latched at 0x14) */
//         }
//     }

//     struct gt911_point_t points[GT911_MAX_TOUCHES] = {0};
//     uint8_t count = 0;


//     while (1) {
//         if (gt911_irq_flag) {
//             gt911_irq_flag = 0;

//             /* Reads 0x814E + coordinates and writes 0x00 to 0x814E so INT stops pulsing */
//             if (gt911_read_points(points, &count) == 0 && count > 0) {
//                 for (uint8_t i = 0; i < count; i++) {
//                     uint8_t  id   = points[i].id;
//                     uint16_t x    = points[i].x;
//                     uint16_t y    = points[i].y;
//                     uint16_t size = points[i].size;

//                     /* Process touch coordinates (x, y) here */
//                     (void)id; (void)x; (void)y; (void)size;
//                 }
//             }
//         } else {
//             /* Sleep CPU until EXTI5 (touch) or SysTick (1 ms) fires */
//             __asm__ volatile ("wfi");
//         }
//     }

//     return 0;
// }