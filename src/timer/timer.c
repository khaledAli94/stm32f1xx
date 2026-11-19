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


/*

#ifndef SYSTICK_H
#define SYSTICK_H

void systick_init(void);

unsigned int millis(void);
void delay(unsigned int ms);

#endif



#include "stm32f10x.h"
#include "systick.h"

static volatile unsigned int ms_ticks = 0;

void systick_init(void)
{
    // 72 MHz / 72000 = 1 kHz (1 ms)
    SysTick->LOAD = 72000 - 1;
    SysTick->VAL  = 0;

    SysTick->CTRL =
        SysTick_CTRL_CLKSOURCE_Msk |
        SysTick_CTRL_TICKINT_Msk   |
        SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Handler(void)
{
    ms_ticks++;
}

unsigned int millis(void)
{
    return ms_ticks;
}

void delay(unsigned int ms)
{
    unsigned int start = millis();

    while ((unsigned int)(millis() - start) < ms)
    {
    }
}


#ifndef TIM2_H
#define TIM2_H

void tim2_init(void);

unsigned short micros(void);
void delay_micros(unsigned short us);

#endif



#include "stm32f10x.h"
#include "tim2.h"

void tim2_init(void)
{
    // Enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // 72 MHz / 72 = 1 MHz
    TIM2->PSC = 72 - 1;

    // 16-bit counter
    TIM2->ARR = 0xFFFF;

    TIM2->CNT = 0;

    // Load prescaler
    TIM2->EGR = TIM_EGR_UG;

    // Clear flags
    TIM2->SR = 0;

    // Start timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

unsigned short micros(void)
{
    return (unsigned short)TIM2->CNT;
}

void delay_micros(unsigned short us)
{
    unsigned short start = micros();

    while ((unsigned short)(micros() - start) < us)
    {
    }
}



 */