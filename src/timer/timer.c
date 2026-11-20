#include <timer.h>
#include <rcc.h>
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

void tim2_init(void)
{
    rcc_tim2_enable();

    /* APB1 timer clock is PCLK1 * 2 whenever APB1 prescaler != 1 */
    uint32_t pclk1   = rcc_get_pclk1_frequency();
    uint32_t tim_clk = (pclk1 == rcc_get_hclk_frequency()) ? pclk1 : (pclk1 * 2U);

    TIM2->CR1 = 0U;
    TIM2->PSC = (tim_clk / 1000000U) - 1U; // 1 tick = 1 us
    TIM2->ARR = 0xFFFFU;                   // Free-running 16-bit wrap
    TIM2->CNT = 0U;
    TIM2->EGR = 1U;                        // UG: Latch prescaler immediately
    TIM2->SR  = 0U;                        // Clear update flag
    TIM2->CR1 = 1U;                        // CEN: Start hardware counter
}

uint16_t micros(void)
{
    return (uint16_t)TIM2->CNT;
}

void delay_micros(uint16_t us)
{
    uint16_t start = micros();
    while ((uint16_t)(micros() - start) < us) {
        /* spin */
    }
}


/*
 tim2 benchmark test
 int main(void)
{
    rcc_sysclk_hse_72mhz();
    systick_init();
    timer_init();
    __asm__ volatile ("cpsie i");

    delay_ms(50);
    delay_us(150);

    uint32_t t_10ms  = millis();
    uint16_t t_500us = micros();

    while (1) {
        if ((uint32_t)(millis() - t_10ms) >= 10U) {
            t_10ms += 10U; // Drift-free 10 ms task
        }

        if ((uint16_t)(micros() - t_500us) >= 500U) {
            t_500us += 500U; // Drift-free 500 us task
        }
    }
}
*/