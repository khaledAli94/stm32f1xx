#ifndef EXTERNAL_INTR_H
#define EXTERNAL_INTR_H

#include <stdint.h>

#define EXTI_BASE 0x40010400UL
#define EXTI ((volatile struct exti_t *) EXTI_BASE)

struct exti_t {
    volatile uint32_t IMR;     // 0x00
    volatile uint32_t EMR;     // 0x04
    volatile uint32_t RTSR;    // 0x08
    volatile uint32_t FTSR;    // 0x0C
    volatile uint32_t SWIER;   // 0x10
    volatile uint32_t PR;      // 0x14
};

#endif /* EXTERNAL_INTR_H */
