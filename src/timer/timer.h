#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIM1_BASE 0x40012C00UL
#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL

#define TIM1 ((volatile struct timer1_t *) TIM1_BASE)
#define TIM2 ((volatile struct timer_t *) TIM2_BASE)
#define TIM3 ((volatile struct timer_t *) TIM3_BASE)
#define TIM4 ((volatile struct timer_t *) TIM4_BASE)

struct timer1_t{
    volatile uint32_t CR1;        // 0x00 Control Register 1
    volatile uint32_t CR2;        // 0x04 Control Register 2
    volatile uint32_t SMCR;       // 0x08 Slave Mode Control Register
    volatile uint32_t DIER;       // 0x0C DMA/Interrupt Enable Register
    volatile uint32_t SR;         // 0x10 Status Register
    volatile uint32_t EGR;        // 0x14 Event Generation Register
    volatile uint32_t CCMR1;      // 0x18 Capture/Compare Mode Register 1
    volatile uint32_t CCMR2;      // 0x1C Capture/Compare Mode Register 2
    volatile uint32_t CCER;       // 0x20 Capture/Compare Enable Register
    volatile uint32_t CNT;        // 0x24 Counter
    volatile uint32_t PSC;        // 0x28 Prescaler
    volatile uint32_t ARR;        // 0x2C Auto‑Reload Register
    volatile uint32_t RCR;        // 0x30 Repetition Counter Register
    volatile uint32_t CCR1;       // 0x34 Capture/Compare Register 1
    volatile uint32_t CCR2;       // 0x38 Capture/Compare Register 2
    volatile uint32_t CCR3;       // 0x3C Capture/Compare Register 3
    volatile uint32_t CCR4;       // 0x40 Capture/Compare Register 4
    volatile uint32_t BDTR;       // 0x44 Break and Dead‑Time Register
    volatile uint32_t DCR;        // 0x48 DMA Control Register
    volatile uint32_t DMAR;       // 0x4C DMA Address for Full Transfer
};

struct timer_t {
    volatile uint32_t CR1;        // 0x00 Control Register 1
    volatile uint32_t CR2;        // 0x04 Control Register 2
    volatile uint32_t SMCR;       // 0x08 Slave Mode Control Register
    volatile uint32_t DIER;       // 0x0C DMA/Interrupt Enable Register
    volatile uint32_t SR;         // 0x10 Status Register
    volatile uint32_t EGR;        // 0x14 Event Generation Register
    volatile uint32_t CCMR1;      // 0x18 Capture/Compare Mode Register 1
    volatile uint32_t CCMR2;      // 0x1C Capture/Compare Mode Register 2
    volatile uint32_t CCER;       // 0x20 Capture/Compare Enable Register
    volatile uint32_t CNT;        // 0x24 Counter
    volatile uint32_t PSC;        // 0x28 Prescaler
    volatile uint32_t ARR;        // 0x2C Auto‑Reload Register
    volatile uint32_t RESERVED0;  // 0x30 (TIM2/3/4 do NOT have RCR here)
    volatile uint32_t CCR1;       // 0x34 Capture/Compare Register 1
    volatile uint32_t CCR2;       // 0x38 Capture/Compare Register 2
    volatile uint32_t CCR3;       // 0x3C Capture/Compare Register 3
    volatile uint32_t CCR4;       // 0x40 Capture/Compare Register 4
    volatile uint32_t RESERVED1;  // 0x44 (TIM2/3/4 do NOT have BDTR here)
    volatile uint32_t DCR;        // 0x48 DMA Control Register
    volatile uint32_t DMAR;       // 0x4C DMA Address Register
};


void tim1_init(void);

 /* TODO : unifiy timers handling */


void tim2_init(void);
uint16_t micros(void);
void delay_micros(uint16_t us);


#endif /* TIMER_H */
