#include <timer.h>
#include <clock.h>
#include <gpio.h>
#include <spi.h>
#include <nested-vector.h>
#include <sys-ctrl-blk.h>

extern volatile unsigned char  _sbss[], _ebss [], _etext[] ;
extern volatile unsigned char   _sdata[] , _edata[];
extern volatile unsigned long  _estack[];
extern void (* const g_pfnVectors[])(void);

void crt0_init(void){
    volatile unsigned long *src, *dst;
    volatile unsigned char *byte_src, *byte_dst;

    SCB->VTOR = (unsigned)g_pfnVectors;

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
    
    tim1_init();
    return 0;
}