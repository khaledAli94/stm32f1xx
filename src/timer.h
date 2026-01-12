#define TIM1_BASE 0x40012C00UL
#define TIM1 ((struct timer_t *) TIM1_BASE)

struct timer_t{
    volatile unsigned CR1;        // 0x00 Control Register 1
    volatile unsigned CR2;        // 0x04 Control Register 2
    volatile unsigned SMCR;       // 0x08 Slave Mode Control Register
    volatile unsigned DIER;       // 0x0C DMA/Interrupt Enable Register
    volatile unsigned SR;         // 0x10 Status Register
    volatile unsigned EGR;        // 0x14 Event Generation Register
    volatile unsigned CCMR1;      // 0x18 Capture/Compare Mode Register 1
    volatile unsigned CCMR2;      // 0x1C Capture/Compare Mode Register 2
    volatile unsigned CCER;       // 0x20 Capture/Compare Enable Register
    volatile unsigned CNT;        // 0x24 Counter
    volatile unsigned PSC;        // 0x28 Prescaler
    volatile unsigned ARR;        // 0x2C Auto‑Reload Register
    volatile unsigned RCR;        // 0x30 Repetition Counter Register
    volatile unsigned CCR1;       // 0x34 Capture/Compare Register 1
    volatile unsigned CCR2;       // 0x38 Capture/Compare Register 2
    volatile unsigned CCR3;       // 0x3C Capture/Compare Register 3
    volatile unsigned CCR4;       // 0x40 Capture/Compare Register 4
    volatile unsigned BDTR;       // 0x44 Break and Dead‑Time Register
    volatile unsigned DCR;        // 0x48 DMA Control Register
    volatile unsigned DMAR;       // 0x4C DMA Address for Full Transfer
} ;

