#include <clock.h>
#include <gpio.h>
#include <spi.h>

/* RCC Driver Functions */
void rcc_hsi_enable(void) {
    /* Enable HSI oscillator - access RCC->CR (offset 0x00) */
    RCC->CR |= (1 << 0);  /* Set HSION bit (bit 0) */
    
    /* Wait for HSI ready flag - access RCC->CR (offset 0x00) */
    while (!(RCC->CR & (1 << 1)));  /* Wait for HSIRDY bit (bit 1) */
}

void rcc_hsi_disable(void) {
    /* Disable HSI oscillator - access RCC->CR (offset 0x00) */
    RCC->CR &= ~(1 << 0);  /* Clear HSION bit (bit 0) */
}

void rcc_hse_enable(void) {
    /* Enable HSE oscillator - access RCC->CR (offset 0x00) */
    RCC->CR |= (1 << 16);  /* Set HSEON bit (bit 16) */
    
    /* Wait for HSE ready flag - access RCC->CR (offset 0x00) */
    while (!(RCC->CR & (1 << 17)));  /* Wait for HSERDY bit (bit 17) */
}

void rcc_hse_disable(void) {
    /* Disable HSE oscillator - access RCC->CR (offset 0x00) */
    RCC->CR &= ~(1 << 16);  /* Clear HSEON bit (bit 16) */
}

void rcc_hse_bypass_enable(int enable) {
    /* Enable or disable HSE bypass - access RCC->CR (offset 0x00) */
    if (enable) {
        RCC->CR |= (1 << 18);  /* Set HSEBYP bit (bit 18) */
    } else {
        RCC->CR &= ~(1 << 18);  /* Clear HSEBYP bit (bit 18) */
    }
}

void rcc_pll_enable(void) {
    /* Enable PLL - access RCC->CR (offset 0x00) */
    RCC->CR |= (1 << 24);  /* Set PLLON bit (bit 24) */
    
    /* Wait for PLL ready flag - access RCC->CR (offset 0x00) */
    while (!(RCC->CR & (1 << 25)));  /* Wait for PLLRDY bit (bit 25) */
}

void rcc_pll_disable(void) {
    /* Disable PLL - access RCC->CR (offset 0x00) */
    RCC->CR &= ~(1 << 24);  /* Clear PLLON bit (bit 24) */
}

void rcc_pll_config(unsigned source, unsigned multiplier) {
    /* Disable PLL before configuration - access RCC->CR (offset 0x00) */
    RCC->CR &= ~(1 << 24);  /* Clear PLLON bit (bit 24) */
    
    /* Wait for PLL to be disabled */
    while (RCC->CR & (1 << 25));  /* Wait for PLLRDY bit (bit 25) to clear */
    
    /* Clear PLL configuration bits - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~((1 << 16) | (0xF << 18));  /* Clear PLLSRC and PLLMUL bits */
    
    /* Set PLL source - access RCC->CFGR (offset 0x04) */
    if (source == 1) {
        RCC->CFGR |= (1 << 16);  /* Set PLLSRC bit (bit 16) to HSE */
    } else {
        RCC->CFGR &= ~(1 << 16);  /* Clear PLLSRC bit (bit 16) to HSI/2 */
    }
    
    /* Set PLL multiplier - access RCC->CFGR (offset 0x04) */
    RCC->CFGR |= ((multiplier & 0xF) << 18);  /* Set PLLMUL[3:0] bits (bits 21:18) */

    /* Note: PLL is still disabled after configuration */
    /* Caller must call rcc_pll_enable() separately */
}

void rcc_sysclk_config(unsigned source) {
    /* Set system clock source - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0x3 << 0);  /* Clear SW[1:0] bits (bits 1:0) */
    RCC->CFGR |= ((source & 0x3) << 0);  /* Set SW[1:0] bits (bits 1:0) */
    
    /* Wait for clock switch */
    while ((RCC->CFGR & (0x3 << 2)) != ((source & 0x3) << 2));  /* Wait for SWS[1:0] (bits 3:2) */
}

void rcc_ahb_prescaler_config(unsigned prescaler) {
    /* Set AHB prescaler - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0xF << 4);  /* Clear HPRE[3:0] bits (bits 7:4) */
    RCC->CFGR |= ((prescaler & 0xF) << 4);  /* Set HPRE[3:0] bits (bits 7:4) */
}

void rcc_apb1_prescaler_config(unsigned prescaler) {
    /* Set APB1 (low speed) prescaler - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0x7 << 8);  /* Clear PPRE1[2:0] bits (bits 10:8) */
    RCC->CFGR |= ((prescaler & 0x7) << 8);  /* Set PPRE1[2:0] bits (bits 10:8) */
}

void rcc_apb2_prescaler_config(unsigned prescaler) {
    /* Set APB2 (high speed) prescaler - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0x7 << 11);  /* Clear PPRE2[2:0] bits (bits 13:11) */
    RCC->CFGR |= ((prescaler & 0x7) << 11);  /* Set PPRE2[2:0] bits (bits 13:11) */
}

void rcc_adc_prescaler_config(unsigned prescaler) {
    /* Set ADC prescaler - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0x3 << 14);  /* Clear ADCPRE[1:0] bits (bits 15:14) */
    RCC->CFGR |= ((prescaler & 0x3) << 14);  /* Set ADCPRE[1:0] bits (bits 15:14) */
}

void rcc_usb_prescaler_config(unsigned prescaler) {
    /* Set USB prescaler - access RCC->CFGR (offset 0x04) */
    if (prescaler) {
        RCC->CFGR |= (1 << 22);  /* Set USBPRE bit (bit 22) for divide by 1.5 */
    } else {
        RCC->CFGR &= ~(1 << 22);  /* Clear USBPRE bit (bit 22) for divide by 1 */
    }
}

void rcc_mco_config(unsigned source) {
    /* Configure MCO (Microcontroller Clock Output) - access RCC->CFGR (offset 0x04) */
    RCC->CFGR &= ~(0x7 << 24);  /* Clear MCO[2:0] bits (bits 26:24) */
    RCC->CFGR |= ((source & 0x7) << 24);  /* Set MCO[2:0] bits (bits 26:24) */
}

/* Peripheral Clock Enable Functions */
void rcc_gpioa_enable(void) {
    /* Enable GPIOA clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 2);  /* Set IOPAEN bit (bit 2) */
}

void rcc_gpiob_enable(void) {
    /* Enable GPIOB clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 3);  /* Set IOPBEN bit (bit 3) */
}

void rcc_gpioc_enable(void) {
    /* Enable GPIOC clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 4);  /* Set IOPCEN bit (bit 4) */
}

void rcc_gpiod_enable(void) {
    /* Enable GPIOD clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 5);  /* Set IOPDEN bit (bit 5) */
}

void rcc_spi1_enable(void) {
    /* Enable SPI1 clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 12);  /* Set SPI1EN bit (bit 12) */
}

void rcc_spi2_enable(void) {
    /* Enable SPI2 clock - access RCC->APB1ENR (offset 0x1C) */
    RCC->APB1ENR |= (1 << 14);  /* Set SPI2EN bit (bit 14) */
}

void rcc_usart1_enable(void) {
    /* Enable USART1 clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 14);  /* Set USART1EN bit (bit 14) */
}

void rcc_usart2_enable(void) {
    /* Enable USART2 clock - access RCC->APB1ENR (offset 0x1C) */
    RCC->APB1ENR |= (1 << 17);  /* Set USART2EN bit (bit 17) */
}

void rcc_i2c1_enable(void) {
    /* Enable I2C1 clock - access RCC->APB1ENR (offset 0x1C) */
    RCC->APB1ENR |= (1 << 21);  /* Set I2C1EN bit (bit 21) */
}

void rcc_i2c2_enable(void) {
    /* Enable I2C2 clock - access RCC->APB1ENR (offset 0x1C) */
    RCC->APB1ENR |= (1 << 22);  /* Set I2C2EN bit (bit 22) */
}

void rcc_tim1_enable(void) {
    /* Enable TIM1 clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 11);  /* Set TIM1EN bit (bit 11) */
}

void rcc_tim2_enable(void) {
    /* Enable TIM2 clock - access RCC->APB1ENR (offset 0x1C) */
    RCC->APB1ENR |= (1 << 0);  /* Set TIM2EN bit (bit 0) */
}

void rcc_dma1_enable(void) {
    /* Enable DMA1 clock - access RCC->AHBENR (offset 0x14) */
    RCC->AHBENR |= (1 << 0);  /* Set DMA1EN bit (bit 0) */
}

void rcc_afio_enable(void) {
    /* Enable AFIO clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 0);  /* Set AFIOEN bit (bit 0) */
}

void rcc_adc1_enable(void) {
    /* Enable ADC1 clock - access RCC->APB2ENR (offset 0x18) */
    RCC->APB2ENR |= (1 << 9);  /* Set ADC1EN bit (bit 9) */
}

/* Peripheral Reset Functions */
void rcc_gpioa_reset(void) {
    /* Reset GPIOA - access RCC->APB2RSTR (offset 0x0C) */
    RCC->APB2RSTR |= (1 << 2);  /* Set IOPARST bit (bit 2) */
    RCC->APB2RSTR &= ~(1 << 2);  /* Clear IOPARST bit (bit 2) */
}

void rcc_spi1_reset(void) {
    /* Reset SPI1 - access RCC->APB2RSTR (offset 0x0C) */
    RCC->APB2RSTR |= (1 << 12);  /* Set SPI1RST bit (bit 12) */
    RCC->APB2RSTR &= ~(1 << 12);  /* Clear SPI1RST bit (bit 12) */
}




/* System Clock Configuration Examples */
void rcc_sysclk_hsi_64mhz(void) {
    /* Configure system clock to 64MHz using HSI */
    
    /* Enable HSI - access RCC->CR (offset 0x00) */
    rcc_hsi_enable();
    
    /* Configure PLL: HSI/2 * 16 = 64MHz - access RCC->CFGR (offset 0x04) */
    rcc_pll_config(0, PLL_MUL_16);  /* Source=HSI/2, Multiplier=16 */
    
    /* Enable PLL - access RCC->CR (offset 0x00) */
    rcc_pll_enable();
    
    /* Configure AHB, APB1, APB2 prescalers - access RCC->CFGR (offset 0x04) */
    rcc_ahb_prescaler_config(AHB_DIV_1);    /* HCLK = 64MHz */
    rcc_apb1_prescaler_config(APB_DIV_2);   /* PCLK1 = 32MHz */
    rcc_apb2_prescaler_config(APB_DIV_1);   /* PCLK2 = 64MHz */
    
    /* Switch to PLL as system clock - access RCC->CFGR (offset 0x04) */
    rcc_sysclk_config(CLOCK_PLL);
}

void rcc_sysclk_hse_72mhz(void) {
    /* Configure system clock to 72MHz using HSE (8MHz crystal) */
    
    /* Enable HSE - access RCC->CR (offset 0x00) */
    rcc_hse_enable();
    
    /* Configure PLL: HSE * 9 = 72MHz - access RCC->CFGR (offset 0x04) */
    rcc_pll_config(1, PLL_MUL_9);  /* Source=HSE, Multiplier=9 */
    
    /* Enable PLL - access RCC->CR (offset 0x00) */
    rcc_pll_enable();
    
    /* Configure AHB, APB1, APB2 prescalers - access RCC->CFGR (offset 0x04) */
    rcc_ahb_prescaler_config(AHB_DIV_1);    /* HCLK = 72MHz */
    rcc_apb1_prescaler_config(APB_DIV_2);   /* PCLK1 = 36MHz */
    rcc_apb2_prescaler_config(APB_DIV_1);   /* PCLK2 = 72MHz */
    
    /* Configure USB prescaler - access RCC->CFGR (offset 0x04) */
    rcc_usb_prescaler_config(USB_DIV_1_5);  /* USB clock = 48MHz */
    
    /* Switch to PLL as system clock - access RCC->CFGR (offset 0x04) */
    rcc_sysclk_config(CLOCK_PLL);
}

void rcc_sysclk_default(void) {
    /* Default system clock configuration (8MHz HSI) */
    
    /* Enable HSI - access RCC->CR (offset 0x00) */
    rcc_hsi_enable();
    
    /* Switch to HSI as system clock - access RCC->CFGR (offset 0x04) */
    rcc_sysclk_config(CLOCK_HSI);
    
    /* Configure AHB, APB1, APB2 prescalers - access RCC->CFGR (offset 0x04) */
    rcc_ahb_prescaler_config(AHB_DIV_1);    /* HCLK = 8MHz */
    rcc_apb1_prescaler_config(APB_DIV_1);   /* PCLK1 = 8MHz */
    rcc_apb2_prescaler_config(APB_DIV_1);   /* PCLK2 = 8MHz */
}

/* Clock Frequency Getter Functions */
unsigned rcc_get_sysclk_frequency(void) {
    unsigned sws = (RCC->CFGR >> 2) & 0x3;  /* Read SWS[1:0] bits (bits 3:2) */
    
    switch (sws) {
        case 0:  /* HSI */
            return 8000000;  /* 8MHz */
        case 1:  /* HSE */
            return 8000000;  /* 8MHz (typical external crystal) */
        case 2:  /* PLL */
            /* Calculate PLL frequency */
            {
                unsigned pllsrc = (RCC->CFGR >> 16) & 0x1;  /* PLLSRC bit (bit 16) */
                unsigned pllmul = (RCC->CFGR >> 18) & 0xF;  /* PLLMUL[3:0] bits (bits 21:18) */
                unsigned pll_input;
                
                if (pllsrc) {
                    pll_input = 8000000;  /* HSE = 8MHz */
                } else {
                    pll_input = 4000000;  /* HSI/2 = 4MHz */
                }
                
                /* PLL multiplier: actual multiplier = pllmul + 2 */
                unsigned multiplier = pllmul + 2;
                if (pllmul == 0xF) {
                    multiplier = 16;  /* Special case: 0xF = 16 */
                }
                
                return pll_input * multiplier;
            }
        default:
            return 8000000;  /* Default to 8MHz */
    }
}

unsigned rcc_get_hclk_frequency(void) {
    unsigned sysclk = rcc_get_sysclk_frequency();
    unsigned hpre = (RCC->CFGR >> 4) & 0xF;  /* HPRE[3:0] bits (bits 7:4) */
    
    switch (hpre) {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
            return sysclk;  /* Not divided */
        case 8: return sysclk / 2;
        case 9: return sysclk / 4;
        case 10: return sysclk / 8;
        case 11: return sysclk / 16;
        case 12: return sysclk / 64;
        case 13: return sysclk / 128;
        case 14: return sysclk / 256;
        case 15: return sysclk / 512;
        default: return sysclk;
    }
}

unsigned rcc_get_pclk1_frequency(void) {
    unsigned hclk = rcc_get_hclk_frequency();
    unsigned ppre1 = (RCC->CFGR >> 8) & 0x7;  /* PPRE1[2:0] bits (bits 10:8) */
    
    switch (ppre1) {
        case 0: case 1: case 2: case 3:
            return hclk;  /* Not divided */
        case 4: return hclk / 2;
        case 5: return hclk / 4;
        case 6: return hclk / 8;
        case 7: return hclk / 16;
        default: return hclk;
    }
}

unsigned rcc_get_pclk2_frequency(void) {
    unsigned hclk = rcc_get_hclk_frequency();
    unsigned ppre2 = (RCC->CFGR >> 11) & 0x7;  /* PPRE2[2:0] bits (bits 13:11) */
    
    switch (ppre2) {
        case 0: case 1: case 2: case 3:
            return hclk;  /* Not divided */
        case 4: return hclk / 2;
        case 5: return hclk / 4;
        case 6: return hclk / 8;
        case 7: return hclk / 16;
        default: return hclk;
    }
}

/* Utility Functions */
void rcc_deinit(void) {
    /* Reset RCC to default state */
    
    /* Turn on HSI - access RCC->CR (offset 0x00) */
    RCC->CR |= (1 << 0);  /* Set HSION bit (bit 0) */
    
    /* Reset CFGR register - access RCC->CFGR (offset 0x04) */
    RCC->CFGR = 0;
    
    /* Clear HSEON, CSSON, PLLON bits - access RCC->CR (offset 0x00) */
    RCC->CR &= ~((1 << 16) | (1 << 19) | (1 << 24));
    
    /* Reset CIR register - access RCC->CIR (offset 0x08) */
    RCC->CIR = 0;
}

/* Example usage for SPI initialization with clocks */
void init_spi1_with_clocks(void) {
    /* Configure system clock to 72MHz using HSE */
    rcc_sysclk_hse_72mhz();
    
    /* Enable GPIOA clock for SPI pins - access RCC->APB2ENR (offset 0x18) */
    rcc_gpioa_enable();
    
    /* Enable SPI1 clock - access RCC->APB2ENR (offset 0x18) */
    rcc_spi1_enable();
    
    /* Configure GPIO pins for SPI1 */
    gpio_pin_mode(GPIOA, PIN_5, GPIO_AF_PP_50MHZ);  /* SCK */
    gpio_pin_mode(GPIOA, PIN_6, GPIO_INPUT_FLOATING); /* MISO */
    gpio_pin_mode(GPIOA, PIN_7, GPIO_AF_PP_50MHZ);  /* MOSI */
    
    /* Now initialize SPI1 */
    spi_init_master(SPI1, SPI_BAUD_DIV16);  /* 72MHz / 16 = 4.5MHz SPI clock */
}

/* Example: Get current clock frequencies */
void print_clock_frequencies(void) {
    unsigned sysclk = rcc_get_sysclk_frequency();
    unsigned hclk = rcc_get_hclk_frequency();
    unsigned pclk1 = rcc_get_pclk1_frequency();
    unsigned pclk2 = rcc_get_pclk2_frequency();
    
    /* For debugging - in real use, you might send these via UART */
    /* sysclk, hclk, pclk1, pclk2 contain frequencies in Hz */
}