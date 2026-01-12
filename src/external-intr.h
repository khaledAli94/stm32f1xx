#define EXTI_BASE 0x40010400UL
#define EXTI ((volatile struct exti_t *) EXTI_BASE)

struct exti_t {
    volatile unsigned IMR;     // 0x00
    volatile unsigned EMR;     // 0x04
    volatile unsigned RTSR;    // 0x08
    volatile unsigned FTSR;    // 0x0C
    volatile unsigned SWIER;   // 0x10
    volatile unsigned PR;      // 0x14
};
