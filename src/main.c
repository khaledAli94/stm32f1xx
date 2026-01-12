#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <nested-vector.h>

extern volatile unsigned char  _sbss[], _ebss [], _etext[] ;
extern volatile unsigned char   _sdata[] , _edata[];
extern volatile unsigned long  _estack[];

void syscpyself(){
    volatile unsigned long *src, *dst;
    volatile unsigned char *byte_src, *byte_dst;
    // Copy initialized data from Flash to RAM (optimize for large chunks)
    src = (unsigned long *) &_etext;
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
    // Enable GPIOC clock (IOPCEN = bit 4) 	
    RCC->APB2ENR |= (1UL<<4); 
    // Configure PC13 as push‑pull output, 2 MHz
    // PC13 is in CRH (pins 8–15)
    GPIOC->CRH &= ~(0xF << 20); // clear CNF13 + MODE13
    GPIOC->CRH |= (0x2 << 20); // MODE13 = 10 (2 MHz), CNF13 = 00 (push‑pull)


    // Enable TIM2 clock (bit 0 on APB1)
    RCC->APB1ENR |= (1 << 0);
    // Timer frequency = 72 MHz / (PSC+1)
    TIM2->PSC = 7200 - 1;      // 10 kHz timer clock
    TIM2->ARR = 10000 - 1;     // overflow every 1 second

    TIM2->DIER |= 1;           // UIE: update interrupt enable
    TIM2->CR1  |= 1;           // CEN: counter enable

    NVIC_EnableIRQ(28);        // TIM2 IRQ number = 28

    return 0;
}