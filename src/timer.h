#define TIM1_BASE 0x40012C00UL
#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL

#define TIM1 ((volatile struct timer1_t *) TIM1_BASE)
#define TIM2 ((volatile struct timer_t *) TIM2_BASE)
#define TIM3 ((volatile struct timer_t *) TIM3_BASE)
#define TIM4 ((volatile struct timer_t *) TIM4_BASE)

struct timer1_t{
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
};

struct timer_t {
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
    volatile unsigned RESERVED0;  // 0x30 (TIM2/3/4 do NOT have RCR here)
    volatile unsigned CCR1;       // 0x34 Capture/Compare Register 1
    volatile unsigned CCR2;       // 0x38 Capture/Compare Register 2
    volatile unsigned CCR3;       // 0x3C Capture/Compare Register 3
    volatile unsigned CCR4;       // 0x40 Capture/Compare Register 4
    volatile unsigned RESERVED1;  // 0x44 (TIM2/3/4 do NOT have BDTR here)
    volatile unsigned DCR;        // 0x48 DMA Control Register
    volatile unsigned DMAR;       // 0x4C DMA Address Register
};


void tim1_init(void);
