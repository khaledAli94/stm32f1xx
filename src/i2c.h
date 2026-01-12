#define I2C1_BASE 0x40005400UL
#define I2C2_BASE 0x40005800UL

#define I2C1 ((volatile struct i2c_t *) I2C1_BASE)
#define I2C2 ((volatile struct i2c_t *) I2C2_BASE)

struct i2c_t {
    volatile unsigned CR1;     // 0x00
    volatile unsigned CR2;     // 0x04
    volatile unsigned OAR1;    // 0x08
    volatile unsigned OAR2;    // 0x0C
    volatile unsigned DR;      // 0x10
    volatile unsigned SR1;     // 0x14
    volatile unsigned SR2;     // 0x18
    volatile unsigned CCR;     // 0x1C
    volatile unsigned TRISE;   // 0x20
};
