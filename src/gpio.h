#define GPIOC_BASE 0x40011000UL
#define GPIOC ((volatile struct gpio_t *) GPIOC_BASE)

struct gpio_t{
    volatile unsigned CRL;     // 0x00
    volatile unsigned CRH;     // 0x04
    volatile unsigned IDR;     // 0x08
    volatile unsigned ODR;     // 0x0C
    volatile unsigned BSRR;    // 0x10
    volatile unsigned BRR;     // 0x14
    volatile unsigned LCKR;    // 0x18
};
