#define ADC1_BASE 0x40012400UL
#define ADC2_BASE 0x40012800UL

#define ADC1 ((volatile struct adc_t *) ADC1_BASE)
#define ADC2 ((volatile struct adc_t *) ADC2_BASE)

struct adc_t {
    volatile unsigned SR;       // 0x00 Status register
    volatile unsigned CR1;      // 0x04 Control register 1
    volatile unsigned CR2;      // 0x08 Control register 2
    volatile unsigned SMPR1;    // 0x0C Sample time register 1
    volatile unsigned SMPR2;    // 0x10 Sample time register 2
    volatile unsigned JOFR1;    // 0x14 Injected channel data offset 1
    volatile unsigned JOFR2;    // 0x18 Injected channel data offset 2
    volatile unsigned JOFR3;    // 0x1C Injected channel data offset 3
    volatile unsigned JOFR4;    // 0x20 Injected channel data offset 4
    volatile unsigned HTR;      // 0x24 Watchdog higher threshold
    volatile unsigned LTR;      // 0x28 Watchdog lower threshold
    volatile unsigned SQR1;     // 0x2C Regular sequence register 1
    volatile unsigned SQR2;     // 0x30 Regular sequence register 2
    volatile unsigned SQR3;     // 0x34 Regular sequence register 3
    volatile unsigned JSQR;     // 0x38 Injected sequence register
    volatile unsigned JDR1;     // 0x3C Injected data register 1
    volatile unsigned JDR2;     // 0x40 Injected data register 2
    volatile unsigned JDR3;     // 0x44 Injected data register 3
    volatile unsigned JDR4;     // 0x48 Injected data register 4
    volatile unsigned DR;       // 0x4C Regular data register
};

