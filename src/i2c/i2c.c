#include <i2c.h>
#include <clock.h>
#include <gpio.h>

static int wait_flag(volatile struct i2c_t *i2c, uint32_t flag)
{
    uint32_t timeout = I2C_TIMEOUT_CYCLES;
    while (!(i2c->SR1 & flag)) {
        if (i2c->SR1 & I2C_SR1_AF) {
            i2c->SR1 &= ~I2C_SR1_AF;
            i2c->CR1 |= I2C_CR1_STOP;
            return I2C_ERR_NACK;
        }
        if (i2c->SR1 & I2C_SR1_ERR_MASK) {
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

static int start_transaction(volatile struct i2c_t *i2c, uint8_t addr_rw)
{
    int ret;
    uint32_t timeout = I2C_TIMEOUT_CYCLES;
    while (i2c->SR2 & I2C_SR2_BUSY) {
        if (--timeout == 0U) return I2C_ERR_BUSY;
    }

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_START;
    if ((ret = wait_flag(i2c, I2C_SR1_SB)) != I2C_OK) return ret;

    i2c->DR = addr_rw;
    if ((ret = wait_flag(i2c, I2C_SR1_ADDR)) != I2C_OK) return ret;

    (void)i2c->SR1; // Clear ADDR sequence
    (void)i2c->SR2;
    return I2C_OK;
}

void i2c_init(volatile struct i2c_t *i2c, uint32_t speed_hz)
{
    uint32_t pclk1_hz  = rcc_get_pclk1_frequency();
    uint32_t pclk1_mhz = pclk1_hz / 1000000U;

    rcc_gpiob_enable();
    rcc_afio_enable();

    if (i2c == I2C1) {
        rcc_i2c1_enable();
        gpio_pin_mode(GPIOB, PIN_6, GPIO_AF_OD_50MHZ);
        gpio_pin_mode(GPIOB, PIN_7, GPIO_AF_OD_50MHZ);
    } else {
        rcc_i2c2_enable();
        gpio_pin_mode(GPIOB, PIN_10, GPIO_AF_OD_50MHZ);
        gpio_pin_mode(GPIOB, PIN_11, GPIO_AF_OD_50MHZ);
    }

    i2c->CR1 = I2C_CR1_SWRST;
    i2c->CR1 = 0;
    i2c->CR2 = pclk1_mhz & 0x3FU;

    if (speed_hz <= 100000U) {
        uint32_t ccr = pclk1_hz / (2U * speed_hz);
        i2c->CCR   = (ccr < 4U) ? 4U : ccr;
        i2c->TRISE = pclk1_mhz + 1U;
    } else {
        uint32_t ccr = pclk1_hz / (3U * speed_hz);
        i2c->CCR   = I2C_CCR_FS | ((ccr < 1U) ? 1U : ccr);
        i2c->TRISE = ((pclk1_mhz * 300U) / 1000U) + 1U;
    }

    i2c->OAR1 = (1U << 14);
    i2c->CR1 |= I2C_CR1_PE;
}

int i2c_write(volatile struct i2c_t *i2c, uint8_t addr, const uint8_t *buf, uint16_t len)
{
    int ret;
    if ((ret = start_transaction(i2c, (uint8_t)(addr << 1))) != I2C_OK) return ret;

    for (uint16_t i = 0; i < len; i++) {
        if ((ret = wait_flag(i2c, I2C_SR1_TXE)) != I2C_OK) return ret;
        i2c->DR = buf[i];
    }

    if ((ret = wait_flag(i2c, I2C_SR1_BTF)) != I2C_OK) return ret;
    i2c->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

int i2c_read(volatile struct i2c_t *i2c, uint8_t addr, uint8_t *buf, uint16_t len)
{
    int ret;
    if (len == 0U) return I2C_OK;

    uint32_t timeout = I2C_TIMEOUT_CYCLES;
    while (i2c->SR2 & I2C_SR2_BUSY) {
        if (--timeout == 0U) return I2C_ERR_BUSY;
    }

    i2c->CR1 &= ~I2C_CR1_POS;
    i2c->CR1 |= I2C_CR1_ACK | I2C_CR1_START;
    if ((ret = wait_flag(i2c, I2C_SR1_SB)) != I2C_OK) return ret;

    i2c->DR = (uint8_t)((addr << 1) | 1U);
    if ((ret = wait_flag(i2c, I2C_SR1_ADDR)) != I2C_OK) return ret;

    if (len == 1U) {
        i2c->CR1 &= ~I2C_CR1_ACK;
        (void)i2c->SR1; (void)i2c->SR2;
        i2c->CR1 |= I2C_CR1_STOP;
        if ((ret = wait_flag(i2c, I2C_SR1_RXNE)) != I2C_OK) return ret;
        buf[0] = (uint8_t)i2c->DR;
        return I2C_OK;
    }

    if (len == 2U) {
        i2c->CR1 |= I2C_CR1_POS;
        (void)i2c->SR1; (void)i2c->SR2;
        i2c->CR1 &= ~I2C_CR1_ACK;
        if ((ret = wait_flag(i2c, I2C_SR1_BTF)) != I2C_OK) return ret;
        i2c->CR1 |= I2C_CR1_STOP;
        buf[0] = (uint8_t)i2c->DR;
        buf[1] = (uint8_t)i2c->DR;
        i2c->CR1 &= ~I2C_CR1_POS;
        return I2C_OK;
    }

    /* len >= 3 */
    (void)i2c->SR1; (void)i2c->SR2;
    for (uint16_t i = 0; i < len - 3U; i++) {
        if ((ret = wait_flag(i2c, I2C_SR1_RXNE)) != I2C_OK) return ret;
        buf[i] = (uint8_t)i2c->DR;
    }

    /* Final 3 bytes */
    if ((ret = wait_flag(i2c, I2C_SR1_BTF)) != I2C_OK) return ret;
    i2c->CR1 &= ~I2C_CR1_ACK;
    buf[len - 3U] = (uint8_t)i2c->DR;
    i2c->CR1 |= I2C_CR1_STOP;
    buf[len - 2U] = (uint8_t)i2c->DR;
    if ((ret = wait_flag(i2c, I2C_SR1_RXNE)) != I2C_OK) return ret;
    buf[len - 1U] = (uint8_t)i2c->DR;

    return I2C_OK;
}