#ifndef EXTERNAL_INTR_H
#define EXTERNAL_INTR_H

#include <stdint.h>

#define EXTI_BASE 0x40010400UL
#define EXTI ((volatile struct exti_t *) EXTI_BASE)

struct exti_t {
    volatile uint32_t IMR;     // 0x00 Interrupt mask register 
    volatile uint32_t EMR;     // 0x04 Event mask register
    volatile uint32_t RTSR;    // 0x08 Rising trigger selection register 
    volatile uint32_t FTSR;    // 0x0C Falling trigger selection register
    volatile uint32_t SWIER;   // 0x10 Software interrupt event register 
    volatile uint32_t PR;      // 0x14  Pending register
};

#endif /* EXTERNAL_INTR_H */
