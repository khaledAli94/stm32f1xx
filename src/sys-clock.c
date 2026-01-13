#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <flash.h>

 /* period = (reload+1)/clk */
/* SysTick Registers */
void sys_tick_init(void){
    // Enable GPIOC clock (IOPCEN = bit 4) 	
    RCC->APB2ENR |= (1 << 4); 
    // Configure PC13 as push‑pull output, 2 MHz
    // PC13 is in CRH (pins 8–15)
    GPIOC->CRH &= ~(1 << 20); // clear CNF13 + MODE13 0xf
    GPIOC->CRH |= (1 << 21); // MODE13 = 10 (2 MHz), CNF13 = 00 (push‑pull) 0x2 21


    SYSTICK->LOAD = 4000000U - 1;
    SYSTICK->VAL = 0;

    /* Enable SysTick:
     * bit 2 CLKSOURCE = (1 = Processor clock, 0 = CPU/8 ) 
     * bit 1 TICKINT =  (Enable interrupt)
     * bit 0 ENABLE =  (Start the timer)
     */
    SYSTICK->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}