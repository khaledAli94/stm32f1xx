#define SPI1_BASE 0x40013000UL
#define SPI2_BASE 0x40003800UL

#define SPI1 ((volatile struct spi_t *) SPI1_BASE)
#define SPI2 ((volatile struct spi_t *) SPI2_BASE)


struct spi_t {
    volatile unsigned CR1;     // 0x00 - Control Register 1
                               // Main SPI configuration register.
                               // Bits:
                               // 0  = CPHA (Clock Phase)
                               // 1  = CPOL (Clock Polarity)
                               // 2  = MSTR (Master Selection: 0=Slave, 1=Master)
                               // 3:5 = BR[2:0] (Baud Rate Control: fPCLK/2^(BR+1))
                               // 6  = SPE (SPI Enable: 0=Disabled, 1=Enabled)
                               // 7  = LSBFIRST (Frame Format: 0=MSB first, 1=LSB first)
                               // 8  = SSI (Internal Slave Select - for master mode)
                               // 9  = SSM (Software Slave Management)
                               // 10 = RXONLY (Receive Only: 0=Full duplex, 1=Receive only)
                               // 11 = DFF (Data Frame Format: 0=8-bit, 1=16-bit)
                               // 12 = CRCNEXT (CRC Transfer Next)
                               // 13 = CRCEN (Hardware CRC Calculation Enable)
                               // 14 = BIDIOE (Bidirectional Output Enable)
                               // 15 = BIDIMODE (Bidirectional Data Mode Enable: 0=2-line, 1=1-line)

    volatile unsigned CR2;     // 0x04 - Control Register 2
                               // DMA and interrupt control register.
                               // Bits:
                               // 0  = RXDMAEN (Rx Buffer DMA Enable)
                               // 1  = TXDMAEN (Tx Buffer DMA Enable)
                               // 2  = SSOE (SS Output Enable: 0=SS output disabled, 1=SS output enabled)
                               // 4  = FRF (Frame Format: 0=Motorola, 1=TI)
                               // 5  = ERRIE (Error Interrupt Enable)
                               // 6  = RXNEIE (RX Buffer Not Empty Interrupt Enable)
                               // 7  = TXEIE (Tx Buffer Empty Interrupt Enable)

    volatile unsigned SR;      // 0x08 - Status Register
                               // SPI status flags.
                               // Bits:
                               // 0  = RXNE (Receive Buffer Not Empty: 0=Empty, 1=Data ready)
                               // 1  = TXE (Transmit Buffer Empty: 0=Not empty, 1=Empty)
                               // 2  = CHSIDE (Channel Side - for TI mode only)
                               // 3  = UDR (Underrun Flag - TI mode only)
                               // 4  = CRCERR (CRC Error Flag: 0=CRC OK, 1=CRC error)
                               // 5  = MODF (Mode Fault: 0=No fault, 1=Fault detected)
                               // 6  = OVR (Overrun Flag: 0=No overrun, 1=Overrun occurred)
                               // 7  = BSY (Busy Flag: 0=SPI not busy, 1=SPI busy)
                               // 8  = FRE (Frame Format Error: 0=No error, 1=Error)

    volatile unsigned DR;      // 0x0C - Data Register
                               // Data transmission and reception register.
                               // Bits:
                               // 0:15 = DR[15:0] (Data bits - actual width depends on DFF bit)
                               // Note: 8-bit mode uses bits [7:0], 16-bit mode uses [15:0]

    volatile unsigned CRCPR;   // 0x10 - CRC Polynomial Register
                               // Defines the polynomial for CRC calculation.
                               // Bits:
                               // 0:15 = CRCPOLY[15:0] (CRC Polynomial value)
                               // Default: 0x0007 (CRC-16 polynomial: x¹⁶ + x¹⁵ + x² + 1)

    volatile unsigned RXCRCR;  // 0x14 - RX CRC Register
                               // Contains the CRC value for received data.
                               // Bits:
                               // 0:15 = RXCRC[15:0] (Received CRC value)
                               // Note: Contains CRC of last received block when CRCEN=1

    volatile unsigned TXCRCR;  // 0x18 - TX CRC Register
                               // Contains the CRC value for transmitted data.
                               // Bits:
                               // 0:15 = TXCRC[15:0] (Transmitted CRC value)
                               // Note: Contains CRC of last transmitted block when CRCEN=1
};


struct spi1_t {
    volatile unsigned CR1;     // 0x00
    volatile unsigned CR2;     // 0x04
    volatile unsigned SR;      // 0x08
    volatile unsigned DR;      // 0x0C
    volatile unsigned CRCPR;   // 0x10
    volatile unsigned RXCRCR;  // 0x14
    volatile unsigned TXCRCR;  // 0x18
};

/* Baud rate prescaler constants */
#define SPI_BAUD_DIV2     0
#define SPI_BAUD_DIV4     1
#define SPI_BAUD_DIV8     2
#define SPI_BAUD_DIV16    3
#define SPI_BAUD_DIV32    4
#define SPI_BAUD_DIV64    5
#define SPI_BAUD_DIV128   6
#define SPI_BAUD_DIV256   7

/* SPI Driver Functions */
void spi_init_master(volatile struct spi_t *spi, unsigned baud_rate);
void spi_init_slave(volatile struct spi_t *spi);
void spi_set_mode(volatile struct spi_t *spi, int cpol, int cpha);
void spi_set_data_size(volatile struct spi_t *spi, int is_16bit);
unsigned char spi_transfer_byte(volatile struct spi_t *spi, unsigned char data);
unsigned short spi_transfer_short(volatile struct spi_t *spi, unsigned short data);
void spi_transmit_bytes(volatile struct spi_t *spi, unsigned char *data, int length);
void spi_receive_bytes(volatile struct spi_t *spi, unsigned char *buffer, int length);
int spi_is_busy(volatile struct spi_t *spi);
int spi_tx_buffer_empty(volatile struct spi_t *spi);
int spi_rx_buffer_ready(volatile struct spi_t *spi);
void spi_enable(volatile struct spi_t *spi);
void spi_disable(volatile struct spi_t *spi);
void spi_set_lsb_first(volatile struct spi_t *spi, int lsb_first);