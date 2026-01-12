#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK ((struct systick_t *) SYSTICK_BASE)

#define RCC_BASE 0x40021000UL
#define RCC ((struct rcc_t *) RCC_BASE)

struct systick_t {
    volatile unsigned CTRL;     // 0x00 Control and Status
    volatile unsigned LOAD;     // 0x04 Reload value
    volatile unsigned VAL;      // 0x08 Current value
    volatile unsigned CALIB;    // 0x0C Calibration value
};

struct rcc_t {
    volatile unsigned CR;          // 0x00
    volatile unsigned CFGR;        // 0x04
    volatile unsigned CIR;         // 0x08
    volatile unsigned APB2RSTR;    // 0x0C
    volatile unsigned APB1RSTR;    // 0x10
    volatile unsigned AHBENR;      // 0x14
    volatile unsigned APB2ENR;     // 0x18
    volatile unsigned APB1ENR;     // 0x1C
    volatile unsigned BDCR;        // 0x20
    volatile unsigned CSR;         // 0x24
};