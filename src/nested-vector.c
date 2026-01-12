#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <nested-vector.h>

void TIM1_UP_IRQHandler(void) {
    TIM1->SR &= ~1;
    GPIOC->ODR ^= (1 << 13);
}

void NVIC_EnableIRQ(unsigned irq) {
    NVIC->ISER[irq >> 5] = (1U << (irq & 0x1F));
}
