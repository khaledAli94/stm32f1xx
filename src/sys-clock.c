/* SysTick Registers */
struct systick_t {
    unsigned CTRL;   // SysTick Control and Status Register
    unsigned LOAD;   // SysTick Reload Value Register
    unsigned VAL;    // SysTick Current Value Register
    unsigned CALIB;  // SysTick Calibration Value Register (optional)
};

void sys_clock_init(void){
    volatile struct systick_t *systk = (struct systick_t*)0xe000e010;
    systk->LOAD = 16000000UL - 1;
    systk->VAL = 0;

    /* Enable SysTick:
     * CLKSOURCE = 1 (Processor clock)
     * TICKINT = 1 (Enable interrupt)
     * ENABLE = 1 (Start the timer)
     */
    systk->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}

int main(){
    volatile unsigned * RCC = (unsigned*)0x40021018;
    volatile unsigned * CRH = (unsigned*)0x40011004;

    *RCC |= (1UL<<4);
	*CRH &=~(1UL<<20);
	*CRH |=(1UL<<21);

    return 0;
}