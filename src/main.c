#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <spi.h>
#include <nested-vector.h>
#include <sys-ctrl-blk.h>
#include <usb-regs.h>

extern volatile unsigned char  _sbss[], _ebss [], _sidata[] ;
extern volatile unsigned char   _sdata[] , _edata[];
extern volatile unsigned long  _estack[];
extern void (* const g_pfnVectors[])(void);

void crt0_init(void){
    volatile unsigned long *src, *dst;
    volatile unsigned char *byte_src, *byte_dst;

    SCB->VTOR = (unsigned)g_pfnVectors;

    // Copy initialized data from Flash to RAM (optimize for large chunks)
    src = (unsigned long *)&_sidata;
    dst = (unsigned long *) &_sdata;
    while (dst + 1 <= (unsigned long *) &_edata) {  *dst++ = *src++;  }
    // Handle remaining bytes (if the section size isn't a multiple of word size)
    byte_src = (unsigned char *) src;
    byte_dst = (unsigned char *) dst;
    while (byte_dst < (unsigned char *) &_edata) { *byte_dst++ = *byte_src++;  }
    // Clear the .bss section (optimize for large chunks)
    dst = (unsigned long *) &_sbss;
    while (dst + 1 <= (unsigned long *) &_ebss) { *dst++ = 0; }
    // Handle remaining bytes in .bss
    byte_dst = (unsigned char *) dst;
    while (byte_dst < (unsigned char *) &_ebss) { *byte_dst++ = 0; }
}

int main(){

    /* Enable GPIOC clock */
    RCC->APB2ENR |= (1U << 4);  /* IOPCEN */

    /* Configure PC13 as Output Push-Pull 2MHz */
    GPIOC->CRH = (GPIOC->CRH & ~0x00F00000U) | 0x00200000U;

    /* Default LED state: OFF (active low -> set high) */
    GPIOC->BSRR = (1U << 13);

    usb_clock_init();
    __asm__("cpsie i");  /* Enable global interrupts */
    usb_init();

    
    while (1)
    {
        __asm__("wfi");  /* wakeup on intterupt */
    }
    

    return 0;
}