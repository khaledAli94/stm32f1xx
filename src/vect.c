#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <nested-vector.h>



void NVIC_EnableIRQ(unsigned irq) {
    NVIC->ISER[irq >> 5] = (1U << (irq & 0x1F));
}


void NVIC_DisableIRQ(unsigned irq) {
    NVIC->ICER[irq >> 5] = (1U << (irq & 0x1F));
}