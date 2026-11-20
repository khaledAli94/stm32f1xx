#include <timer.h>
#include <rcc.h>
#include <systick.h>
#include <gpio.h>
#include <flash-regs.h>

/* period = (reload+1)/clk */

volatile uint32_t ms_ticks = 0;

void systick_init(void)
{
    SYSTICK->LOAD = (rcc_get_sysclk_frequency() / 1000U) - 1U;
    SYSTICK->VAL  = 0U;
    
    /* Enable SysTick:
     * bit 2 CLKSOURCE = (1 = Processor clock, 0 = CPU/8 ) 
     * bit 1 TICKINT =  (Enable interrupt)
     * bit 0 ENABLE =  (Start the timer)
     */
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

uint32_t millis(void)
{
    return ms_ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = ms_ticks;
    while ((uint32_t)(ms_ticks - start) < ms) {
        __asm__ volatile ("wfi");
    }
}