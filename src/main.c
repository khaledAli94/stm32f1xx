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
    init_spi1_with_clocks();
    
    // Set SPI mode 0 (CPOL=0, CPHA=0)
    spi_set_mode(SPI1, 0, 0);
    
    // Set MSB first (default)
    spi_set_lsb_first(SPI1, 0);
    
    // Set 8-bit data size (default)
    spi_set_data_size(SPI1, 0);
    
    // Transfer a byte
    unsigned char received = spi_transfer_byte(SPI1, 0x55);
    
    // Send array of bytes
    unsigned char tx_data[] = {0x01, 0x02, 0x03};
    spi_transmit_bytes(SPI1, tx_data, 3);
    
    // Receive array of bytes
    unsigned char rx_data[3];
    spi_receive_bytes(SPI1, rx_data, 3);



    tim1_init();
    return 0;
}