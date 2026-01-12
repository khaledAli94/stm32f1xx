#define SPI1_BASE 0x40013000UL
#define SPI2_BASE 0x40003800UL

#define SPI1 ((volatile struct spi_t *) SPI1_BASE)
#define SPI2 ((volatile struct spi_t *) SPI2_BASE)


struct spi_t {
    volatile unsigned CR1;     // 0x00
    volatile unsigned CR2;     // 0x04
    volatile unsigned SR;      // 0x08
    volatile unsigned DR;      // 0x0C
    volatile unsigned CRCPR;   // 0x10
    volatile unsigned RXCRCR;  // 0x14
    volatile unsigned TXCRCR;  // 0x18
};

