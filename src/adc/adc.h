#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define ADC1_BASE 0x40012400UL
#define ADC2_BASE 0x40012800UL

#define ADC1 ((volatile struct adc_t *) ADC1_BASE)
#define ADC2 ((volatile struct adc_t *) ADC2_BASE)

struct adc_t {
    volatile uint32_t SR;       // 0x00 Status register
    volatile uint32_t CR1;      // 0x04 Control register 1
    volatile uint32_t CR2;      // 0x08 Control register 2
    volatile uint32_t SMPR1;    // 0x0C Sample time register 1
    volatile uint32_t SMPR2;    // 0x10 Sample time register 2
    volatile uint32_t JOFR1;    // 0x14 Injected channel data offset 1
    volatile uint32_t JOFR2;    // 0x18 Injected channel data offset 2
    volatile uint32_t JOFR3;    // 0x1C Injected channel data offset 3
    volatile uint32_t JOFR4;    // 0x20 Injected channel data offset 4
    volatile uint32_t HTR;      // 0x24 Watchdog higher threshold
    volatile uint32_t LTR;      // 0x28 Watchdog lower threshold
    volatile uint32_t SQR1;     // 0x2C Regular sequence register 1
    volatile uint32_t SQR2;     // 0x30 Regular sequence register 2
    volatile uint32_t SQR3;     // 0x34 Regular sequence register 3
    volatile uint32_t JSQR;     // 0x38 Injected sequence register
    volatile uint32_t JDR1;     // 0x3C Injected data register 1
    volatile uint32_t JDR2;     // 0x40 Injected data register 2
    volatile uint32_t JDR3;     // 0x44 Injected data register 3
    volatile uint32_t JDR4;     // 0x48 Injected data register 4
    volatile uint32_t DR;       // 0x4C Regular data register
};

#endif /* ADC_H */
