#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <nested-vector.h>

void tim1_init(void) {
    // Enable clocks
    RCC->APB2ENR |= (1 << 11);   // TIM1 clock

    // Configure PC13 as push‑pull output
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |=  (0x1 << 20);  // MODE13 = 01 (10 MHz output)
                                 // CNF13  = 00 (push‑pull)

    // Timer setup
    TIM1->PSC = 7200 - 1;        // 10 kHz timer clock
    TIM1->ARR = 10000 - 1;       // 1 Hz overflow
    TIM1->DIER |= 1;             // UIE: update interrupt enable
    TIM1->CR1 |= 1;              // CEN: counter enable

    NVIC_EnableIRQ(TIM1_UP_IRQn);
}