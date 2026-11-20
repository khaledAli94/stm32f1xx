#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK ((struct systick_t *) SYSTICK_BASE)

struct systick_t {
    volatile uint32_t CTRL;     // 0x00 Control and Status
    volatile uint32_t LOAD;     // 0x04 Reload value
    volatile uint32_t VAL;      // 0x08 Current value
    volatile uint32_t CALIB;    // 0x0C Calibration value
};

#define SYSTICK_CTRL_ENABLE     (1U << 0)
#define SYSTICK_CTRL_TICKINT    (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1U << 2)

extern volatile uint32_t ms_ticks;

void systick_init(void);
uint32_t millis(void);
void delay_ms(uint32_t ms);

#endif /* SYSTICK_H */
