#include <spi.h>

/* SPI Driver Functions */
void spi_init_master(volatile struct spi_t *spi, unsigned baud_rate) {
    /* Disable SPI before configuration - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
    
    /* Reset all configurations - access spi->CR1 (offset 0x00) */
    spi->CR1 = 0;
    
    /* Reset CR2 - access spi->CR2 (offset 0x04) */
    spi->CR2 = 0;
    
    /* Configure as master mode - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 2);   /* Set MSTR bit (bit 2) */
    
    /* Enable software slave management - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 9);   /* Set SSM bit (bit 9) */
    
    /* Set internal slave select high - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 8);   /* Set SSI bit (bit 8) */
    
    /* Set baud rate prescaler - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (baud_rate & 0x7) << 3;  /* Set BR[2:0] bits (bits 5:3) */
    
    /* Enable SPI peripheral - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);   /* Set SPE bit (bit 6) */
}

void spi_init_slave(volatile struct spi_t *spi) {
    /* Disable SPI before configuration - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
    
    /* Reset all configurations - access spi->CR1 (offset 0x00) */
    spi->CR1 = 0;
    
    /* Reset CR2 - access spi->CR2 (offset 0x04) */
    spi->CR2 = 0;
    
    /* Configure as slave mode (MSTR=0 by default) */
    /* Enable software slave management - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 9);   /* Set SSM bit (bit 9) */
    
    /* Enable SPI peripheral - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);   /* Set SPE bit (bit 6) */
}

void spi_set_mode(volatile struct spi_t *spi, int cpol, int cpha) {
    /* Disable SPI before changing mode - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
    
    /* Clear CPOL and CPHA bits - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~((1 << 1) | (1 << 0));  /* Clear bits 1 and 0 */
    
    /* Set CPOL (Clock Polarity) if requested - access spi->CR1 (offset 0x00) */
    if (cpol) {
        spi->CR1 |= (1 << 1);  /* Set CPOL bit (bit 1) */
    }
    
    /* Set CPHA (Clock Phase) if requested - access spi->CR1 (offset 0x00) */
    if (cpha) {
        spi->CR1 |= (1 << 0);  /* Set CPHA bit (bit 0) */
    }
    
    /* Re-enable SPI - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);   /* Set SPE bit (bit 6) */
}

void spi_set_data_size(volatile struct spi_t *spi, int is_16bit) {
    /* Disable SPI before changing data size - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
    
    /* Set or clear DFF bit (Data Frame Format) - access spi->CR1 (offset 0x00) */
    if (is_16bit) {
        spi->CR1 |= (1 << 11);  /* Set DFF bit (bit 11) for 16-bit data */
    } else {
        spi->CR1 &= ~(1 << 11); /* Clear DFF bit (bit 11) for 8-bit data */
    }
    
    /* Re-enable SPI - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);   /* Set SPE bit (bit 6) */
}

unsigned char spi_transfer_byte(volatile struct spi_t *spi, unsigned char data) {
    /* Wait until TX buffer is empty - access spi->SR (offset 0x08) */
    while (!(spi->SR & (1 << 1)));  /* Check TXE bit (bit 1) */
    
    /* Write data to transmit - access spi->DR (offset 0x0C) */
    spi->DR = data;
    
    /* Wait until RX buffer has data - access spi->SR (offset 0x08) */
    while (!(spi->SR & (1 << 0)));  /* Check RXNE bit (bit 0) */
    
    /* Return received data - access spi->DR (offset 0x0C) */
    return (unsigned char)(spi->DR & 0xFF);
}

unsigned short spi_transfer_short(volatile struct spi_t *spi, unsigned short data) {
    /* Wait until TX buffer is empty - access spi->SR (offset 0x08) */
    while (!(spi->SR & (1 << 1)));  /* Check TXE bit (bit 1) */
    
    /* Write data to transmit - access spi->DR (offset 0x0C) */
    spi->DR = data;
    
    /* Wait until RX buffer has data - access spi->SR (offset 0x08) */
    while (!(spi->SR & (1 << 0)));  /* Check RXNE bit (bit 0) */
    
    /* Return received data - access spi->DR (offset 0x0C) */
    return (unsigned short)(spi->DR & 0xFFFF);
}

void spi_transmit_bytes(volatile struct spi_t *spi, unsigned char *data, int length) {
    for (int i = 0; i < length; i++) {
        spi_transfer_byte(spi, data[i]);
    }
}

void spi_receive_bytes(volatile struct spi_t *spi, unsigned char *buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] = spi_transfer_byte(spi, 0xFF);  /* Send dummy byte to receive */
    }
}

int spi_is_busy(volatile struct spi_t *spi) {
    /* Check busy flag - access spi->SR (offset 0x08) */
    return (spi->SR & (1 << 7)) ? 1 : 0;  /* Check BSY bit (bit 7) */
}

int spi_tx_buffer_empty(volatile struct spi_t *spi) {
    /* Check TX buffer empty flag - access spi->SR (offset 0x08) */
    return (spi->SR & (1 << 1)) ? 1 : 0;  /* Check TXE bit (bit 1) */
}

int spi_rx_buffer_ready(volatile struct spi_t *spi) {
    /* Check RX buffer ready flag - access spi->SR (offset 0x08) */
    return (spi->SR & (1 << 0)) ? 1 : 0;  /* Check RXNE bit (bit 0) */
}

void spi_enable(volatile struct spi_t *spi) {
    /* Enable SPI - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);  /* Set SPE bit (bit 6) */
}

void spi_disable(volatile struct spi_t *spi) {
    /* Disable SPI - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
}

void spi_set_lsb_first(volatile struct spi_t *spi, int lsb_first) {
    /* Disable SPI before changing format - access spi->CR1 (offset 0x00) */
    spi->CR1 &= ~(1 << 6);  /* Clear SPE bit (bit 6) */
    
    /* Set or clear LSBFIRST bit - access spi->CR1 (offset 0x00) */
    if (lsb_first) {
        spi->CR1 |= (1 << 7);   /* Set LSBFIRST bit (bit 7) */
    } else {
        spi->CR1 &= ~(1 << 7);  /* Clear LSBFIRST bit (bit 7) */
    }
    
    /* Re-enable SPI - access spi->CR1 (offset 0x00) */
    spi->CR1 |= (1 << 6);   /* Set SPE bit (bit 6) */
}


/* Example usage */
/*
int main(void) {
    volatile struct gpio_t *A = gpio_get_port(PORT_A);

    // SPI pins
    gpio_config_alternate(A, PIN_5, 50, 0);   // PA5 SCK, AF PP
    gpio_config_alternate(A, PIN_7, 50, 0);   // PA7 MOSI, AF PP

    // Control pins
    gpio_config_output(A, PIN_4, 50);         // PA4 CS
    gpio_config_output(A, PIN_3, 50);         // PA3 DC
    gpio_config_output(A, PIN_2, 50);         // PA2 RESET

    // Default states
    gpio_pin_set_high(A, PIN_4);  // CS high (inactive)
    gpio_pin_set_high(A, PIN_3);  // DC high (data mode default)
    gpio_pin_set_high(A, PIN_2);  // RESET high

    
    // First enable SPI1 clock in RCC
    // RCC->APB2ENR |= (1 << 12);
    
    // Initialize SPI1 as master with baud rate divider 16
    spi_init_master(SPI1, SPI_BAUD_DIV16);
    
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
    
    return 0;
}
*/