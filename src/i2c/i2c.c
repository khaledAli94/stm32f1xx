#include <i2c.h>
#include <clock.h>
#include <gpio.h>


static int i2c_wait_flag(volatile struct i2c_t *i2c, unsigned flag, int state)
{
    unsigned timeout = I2C_TIMEOUT_CYCLES;
    while ((((i2c->SR1 & flag) != 0U) != state)) {
        if (i2c->SR1 & I2C_SR1_AF) {
            i2c->SR1 &= ~I2C_SR1_AF;
            i2c->CR1 |= I2C_CR1_STOP;
            return I2C_ERR_NACK;
        }
        if (i2c->SR1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)) {
            i2c->SR1 &= ~I2C_SR1_ERR_MASK;
            i2c->CR1 |= I2C_CR1_STOP;
            return I2C_ERR_BUS;
        }
        if (--timeout == 0U) {
            i2c->CR1 |= I2C_CR1_STOP;
            return I2C_ERR_TIMEOUT;
        }
    }
    return I2C_OK;
}

static int i2c_wait_busy(volatile struct i2c_t *i2c)
{
    unsigned timeout = I2C_TIMEOUT_CYCLES;
    while (i2c->SR2 & I2C_SR2_BUSY) {
        if (--timeout == 0U) return I2C_ERR_BUSY;
    }
    return I2C_OK;
}

static inline void i2c_reset(volatile struct i2c_t *i2c)
{
    i2c->CR1 |= I2C_CR1_SWRST;
    i2c->CR1 &= ~I2C_CR1_SWRST;
}

void i2c_init(volatile struct i2c_t *i2c, unsigned speed_hz)
{
    unsigned ccr_val; // we can hardcode it obviously, or by passing a  clk getter!
    unsigned pclk1_hz  = rcc_get_pclk1_frequency();
    unsigned pclk1_mhz = pclk1_hz / 1000000U;

    /* 1. Enable GPIOB, AFIO, and Peripheral Clocks */
    rcc_gpiob_enable();
    rcc_afio_enable();

    if (i2c == I2C1) {
        rcc_i2c1_enable();
        /* PB6 = I2C1_SCL, PB7 = I2C1_SDA (Alternate Function Open-Drain 50MHz) */
        gpio_pin_mode(GPIOB, PIN_6, GPIO_AF_OD_50MHZ);
        gpio_pin_mode(GPIOB, PIN_7, GPIO_AF_OD_50MHZ);
    } else if (i2c == I2C2) {
        rcc_i2c2_enable();
        /* PB10 = I2C2_SCL, PB11 = I2C2_SDA (Alternate Function Open-Drain 50MHz) */
        gpio_pin_mode(GPIOB, PIN_10, GPIO_AF_OD_50MHZ);
        gpio_pin_mode(GPIOB, PIN_11, GPIO_AF_OD_50MHZ);
    }

    /* 2. Disable & Software Reset I2C Peripheral */
    i2c->CR1 &= ~I2C_CR1_PE;
    i2c_reset(i2c);

    /* 3. Configure Input Clock Frequency (36 MHz at 72 MHz SYSCLK) */
    i2c->CR2 = (pclk1_mhz & I2C_CR2_FREQ_MASK);

    /* 4. Configure CCR and TRISE */
    if (speed_hz <= 100000U) {
        /* Standard Mode (100 kHz): CCR = 36MHz / (2 * 100kHz) = 180 */
        ccr_val = pclk1_hz / (2U * speed_hz);
        if (ccr_val < 4U) ccr_val = 4U;
        i2c->CCR   = ccr_val;
        i2c->TRISE = pclk1_mhz + 1U; // 37
    } else {
        /* Fast Mode (400 kHz, Duty 2:1): CCR = 36MHz / (3 * 400kHz) = 30 */
        ccr_val = pclk1_hz / (3U * speed_hz);
        if (ccr_val < 1U) ccr_val = 1U;
        i2c->CCR   = I2C_CCR_FS | ccr_val;
        i2c->TRISE = ((pclk1_mhz * 300U) / 1000U) + 1U; // 11
    }

    /* 5. Enable Peripheral */
    i2c->OAR1 = (1U << 14);
    i2c->CR1 |= I2C_CR1_PE;
}

int i2c_write(volatile struct i2c_t *i2c, unsigned char addr, const unsigned char *data, unsigned short len)
{
    int ret;
    if (i2c_wait_busy(i2c) != I2C_OK) return I2C_ERR_BUSY;

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_START;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_SB, 1)) != I2C_OK) return ret;

    i2c->DR = (unsigned)(addr << 1); // Write (LSB = 0)
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_ADDR, 1)) != I2C_OK) return ret;

    /* Clear ADDR */
    (void)i2c->SR1;
    (void)i2c->SR2;

    for (unsigned short i = 0; i < len; i++) {
        if ((ret = i2c_wait_flag(i2c, I2C_SR1_TXE, 1)) != I2C_OK) return ret;
        i2c->DR = data[i];
    }

    if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
    i2c->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

int i2c_read(volatile struct i2c_t *i2c, unsigned char addr, unsigned char *data, unsigned short len)
{
    int ret;
    if (len == 0U) return I2C_OK;
    if (i2c_wait_busy(i2c) != I2C_OK) return I2C_ERR_BUSY;

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK | I2C_CR1_START;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_SB, 1)) != I2C_OK) return ret;

    i2c->DR = (unsigned)((addr << 1) | 1U); // Read (LSB = 1)
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_ADDR, 1)) != I2C_OK) return ret;

    if (len == 1U) {
        i2c->CR1 &= ~I2C_CR1_ACK;
        (void)i2c->SR1;
        (void)i2c->SR2;
        i2c->CR1 |= I2C_CR1_STOP;

        if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
        data[0] = (unsigned char)i2c->DR;
    } else if (len == 2U) {
        i2c->CR1 |= I2C_CR1_POS;
        (void)i2c->SR1;
        (void)i2c->SR2;
        i2c->CR1 &= ~I2C_CR1_ACK;

        if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
        i2c->CR1 |= I2C_CR1_STOP;
        data[0] = (unsigned char)i2c->DR;
        data[1] = (unsigned char)i2c->DR;
        i2c->CR1 &= ~I2C_CR1_POS;
    } else {
        (void)i2c->SR1;
        (void)i2c->SR2;

        for (unsigned short i = 0; i < len; i++) {
            if (i == (len - 3U)) {
                if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
                i2c->CR1 &= ~I2C_CR1_ACK;
                data[i++] = (unsigned char)i2c->DR; // N-2
                i2c->CR1 |= I2C_CR1_STOP;
                data[i++] = (unsigned char)i2c->DR; // N-1
                if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
                data[i] = (unsigned char)i2c->DR;   // N
                break;
            }
            if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
            data[i] = (unsigned char)i2c->DR;
        }
    }
    return I2C_OK;
}

int i2c_write_reg(volatile struct i2c_t *i2c, unsigned char addr, unsigned char reg, const unsigned char *data, unsigned short len)
{
    int ret;
    if (i2c_wait_busy(i2c) != I2C_OK) return I2C_ERR_BUSY;

    i2c->CR1 |= I2C_CR1_START;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_SB, 1)) != I2C_OK) return ret;

    i2c->DR = (unsigned)(addr << 1);
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_ADDR, 1)) != I2C_OK) return ret;
    (void)i2c->SR1;
    (void)i2c->SR2;

    if ((ret = i2c_wait_flag(i2c, I2C_SR1_TXE, 1)) != I2C_OK) return ret;
    i2c->DR = reg;

    for (unsigned short i = 0; i < len; i++) {
        if ((ret = i2c_wait_flag(i2c, I2C_SR1_TXE, 1)) != I2C_OK) return ret;
        i2c->DR = data[i];
    }

    if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
    i2c->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

int i2c_read_reg(volatile struct i2c_t *i2c, unsigned char addr, unsigned char reg, unsigned char *data, unsigned short len)
{
    int ret;
    if (len == 0U) return I2C_OK;
    if (i2c_wait_busy(i2c) != I2C_OK) return I2C_ERR_BUSY;

    /* Phase 1: Send register address without STOP */
    i2c->CR1 |= I2C_CR1_START;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_SB, 1)) != I2C_OK) return ret;

    i2c->DR = (unsigned)(addr << 1);
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_ADDR, 1)) != I2C_OK) return ret;
    (void)i2c->SR1;
    (void)i2c->SR2;

    if ((ret = i2c_wait_flag(i2c, I2C_SR1_TXE, 1)) != I2C_OK) return ret;
    i2c->DR = reg;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;

    /* Phase 2: Repeated START for Read */
    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK | I2C_CR1_START;
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_SB, 1)) != I2C_OK) return ret;

    i2c->DR = (unsigned)((addr << 1) | 1U);
    if ((ret = i2c_wait_flag(i2c, I2C_SR1_ADDR, 1)) != I2C_OK) return ret;

    if (len == 1U) {
        i2c->CR1 &= ~I2C_CR1_ACK;
        (void)i2c->SR1;
        (void)i2c->SR2;
        i2c->CR1 |= I2C_CR1_STOP;
        if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
        data[0] = (unsigned char)i2c->DR;
    } else if (len == 2U) {
        i2c->CR1 |= I2C_CR1_POS;
        (void)i2c->SR1;
        (void)i2c->SR2;
        i2c->CR1 &= ~I2C_CR1_ACK;
        if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
        i2c->CR1 |= I2C_CR1_STOP;
        data[0] = (unsigned char)i2c->DR;
        data[1] = (unsigned char)i2c->DR;
        i2c->CR1 &= ~I2C_CR1_POS;
    } else {
        (void)i2c->SR1;
        (void)i2c->SR2;
        for (unsigned short i = 0; i < len; i++) {
            if (i == (len - 3U)) {
                if ((ret = i2c_wait_flag(i2c, I2C_SR1_BTF, 1)) != I2C_OK) return ret;
                i2c->CR1 &= ~I2C_CR1_ACK;
                data[i++] = (unsigned char)i2c->DR;
                i2c->CR1 |= I2C_CR1_STOP;
                data[i++] = (unsigned char)i2c->DR;
                if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
                data[i] = (unsigned char)i2c->DR;
                break;
            }
            if ((ret = i2c_wait_flag(i2c, I2C_SR1_RXNE, 1)) != I2C_OK) return ret;
            data[i] = (unsigned char)i2c->DR;
        }
    }
    return I2C_OK;
}