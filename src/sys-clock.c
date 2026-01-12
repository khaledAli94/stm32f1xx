#include <timer.h>
#include <clock.h>
#include <gpio.h>

/* SysTick Registers */
void sys_clock_init(void){
    SYSTICK->LOAD = 16000000U - 1;
    SYSTICK->VAL = 0;

    /* Enable SysTick:
     * CLKSOURCE = 1 (Processor clock)
     * TICKINT = 1 (Enable interrupt)
     * ENABLE = 1 (Start the timer)
     */
    SYSTICK->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}