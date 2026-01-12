#include <timer.h>
#include <clock.h>
#include <gpio.h>

 /* period = (reload+1)/clk */
/* SysTick Registers */
void sys_tick_init(void){
    // Enable GPIOC clock (IOPCEN = bit 4) 	
    RCC->APB2ENR |= (1 << 4); 
    // Configure PC13 as push‑pull output, 2 MHz
    // PC13 is in CRH (pins 8–15)
    GPIOC->CRH &= ~(0xF << 20); // clear CNF13 + MODE13
    GPIOC->CRH |= (0x2 << 20); // MODE13 = 10 (2 MHz), CNF13 = 00 (push‑pull)


    SYSTICK->LOAD = 4500000U - 1;
    SYSTICK->VAL = 0;

    /* Enable SysTick:
     * bit 2 CLKSOURCE = (1 = Processor clock, 0 = CPU/8 ) 
     * bit 1 TICKINT =  (Enable interrupt)
     * bit 0 ENABLE =  (Start the timer)
     */
    SYSTICK->CTRL = (0 << 2) | (1 << 1) | (1 << 0);
}

