#ifndef I2C_H
#define I2C_H

#include <stdint.h>

 /* i2c -> APB, i2c-> gpioB (SCL1->B6|B8 , SDA1->B7|B9, SCL2->10, SDA2->11) 
 i2c DMA I2C2_TX->ch4 , I2C2_RX->ch5, I2C1_TX->ch6, I2C1_RX->ch7  */
#define I2C1_BASE 0x40005400UL
#define I2C2_BASE 0x40005800UL

#define I2C1 ((volatile struct i2c_t *) I2C1_BASE)
#define I2C2 ((volatile struct i2c_t *) I2C2_BASE)

struct i2c_t {
    volatile uint32_t CR1;     // 0x00 ctrl     
    volatile uint32_t CR2;     // 0x04
    volatile uint32_t OAR1;    // 0x08 own addr 
    volatile uint32_t OAR2;    // 0x0C 
    volatile uint32_t DR;      // 0x10 data
    volatile uint32_t SR1;     // 0x14 status 
    volatile uint32_t SR2;     // 0x18 
    volatile uint32_t CCR;     // 0x1C clock ctrl
    volatile uint32_t TRISE;   // 0x20 
};


/* CR1 Bit Definitions */
#define I2C_CR1_PE          (1U << 0)
#define I2C_CR1_START       (1U << 8)
#define I2C_CR1_STOP        (1U << 9)
#define I2C_CR1_ACK         (1U << 10)
#define I2C_CR1_POS         (1U << 11)
#define I2C_CR1_SWRST       (1U << 15)


/* CR2 Bit Definitions */
#define I2C_CR2_FREQ_MASK   (0x3fU << 0)
#define I2C_CR2_ITERREN     (1U << 8)
#define I2C_CR2_ITEVTEN     (1U << 9)
#define I2C_CR2_ITBUFEN     (1U << 10)
#define I2C_CR2_DMAEN       (1U << 11)
#define I2C_CR2_LAST        (1U << 12)

/* SR1 Bit Definitions */
#define I2C_SR1_SB          (1U << 0)
#define I2C_SR1_ADDR        (1U << 1)
#define I2C_SR1_BTF         (1U << 2)
#define I2C_SR1_RXNE        (1U << 6)
#define I2C_SR1_TXE         (1U << 7)
#define I2C_SR1_BERR        (1U << 8)
#define I2C_SR1_ARLO        (1U << 9)
#define I2C_SR1_AF          (1U << 10)
#define I2C_SR1_OVR         (1U << 11)
#define I2C_SR1_TIMEOUT     (1U << 14)
#define I2C_SR1_ERR_MASK    (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_AF | I2C_SR1_OVR | I2C_SR1_TIMEOUT)

/* SR2 Bit Definitions */
#define I2C_SR2_MSL         (1U << 0)
#define I2C_SR2_BUSY        (1U << 1)
#define I2C_SR2_TRA         (1U << 2)

/* CCR Bit Definitions */
#define I2C_CCR_FS          (1U << 15)
#define I2C_CCR_DUTY        (1U << 14)


#define I2C_TIMEOUT_CYCLES 100000U
#define FREQ_400KHZ 400000U
#define FREQ_100KHZ 100000U


/* Driver Status Codes */
enum {
    I2C_OK = 0,
    I2C_ERR_TIMEOUT = -1,
    I2C_ERR_NACK = -2,
    I2C_ERR_BUS = -3,
    I2C_ERR_BUSY = -4
};



void i2c_init(volatile struct i2c_t *i2c, uint32_t speed_hz);
int  i2c_write(volatile struct i2c_t *i2c, uint8_t addr, const uint8_t *buf, uint16_t len);
int  i2c_read(volatile struct i2c_t *i2c, uint8_t addr, uint8_t *buf, uint16_t len);

#endif /* I2C_H */
