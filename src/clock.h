#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK ((struct systick_t *) SYSTICK_BASE)

#define RCC_BASE 0x40021000UL
#define RCC ((struct rcc_t *) RCC_BASE)

struct systick_t {
    volatile unsigned CTRL;     // 0x00 Control and Status
    volatile unsigned LOAD;     // 0x04 Reload value
    volatile unsigned VAL;      // 0x08 Current value
    volatile unsigned CALIB;    // 0x0C Calibration value
};

struct rcc_t {
    volatile unsigned CR;          // 0x00 - Clock Control Register
                                   // Controls HSI, HSE, PLL on/off and ready flags.
                                   // Bits:
                                   // 0  = HSION (enable internal 8 MHz RC)
                                   // 1  = HSIRDY (HSI ready)
                                   // 16 = HSEON (enable external crystal)
                                   // 17 = HSERDY (HSE ready)
                                   // 24 = PLLON (enable PLL)
                                   // 25 = PLLRDY (PLL ready)

    volatile unsigned CFGR;        // 0x04 - Clock Configuration Register
                                   // Selects system clock source and prescalers.
                                   // Bits:
                                   // 1:0   = SW (SYSCLK source: HSI/HSE/PLL)
                                   // 3:2   = SWS (clock status)
                                   // 7:4   = HPRE (AHB prescaler)
                                   // 10:8  = PPRE1 (APB1 prescaler)
                                   // 13:11 = PPRE2 (APB2 prescaler)
                                   // 16    = PLLSRC (PLL source: HSI/2 or HSE)
                                   // 21:18 = PLLMUL (PLL multiplier)

    volatile unsigned CIR;         // 0x08 - Clock Interrupt Register
                                   // Enables/disables clock interrupts and clears flags.
                                   // Bits:
                                   // 0–7   = interrupt flags (HSI, HSE, PLL ready)
                                   // 8–15  = interrupt enables
                                   // 16–23 = interrupt clear bits

    volatile unsigned APB2RSTR;    // 0x0C - APB2 Peripheral Reset Register
                                   // Resets peripherals on APB2 bus.
                                   // Example bits:
                                   // 2 = AFIO reset
                                   // 4 = IOPA reset
                                   // 5 = IOPB reset
                                   // 6 = IOPC reset
                                   // 11 = ADC1 reset
                                   // 14 = USART1 reset

    volatile unsigned APB1RSTR;    // 0x10 - APB1 Peripheral Reset Register
                                   // Resets peripherals on APB1 bus.
                                   // Example bits:
                                   // 0  = TIM2 reset
                                   // 1  = TIM3 reset
                                   // 17 = USART2 reset
                                   // 18 = USART3 reset
                                   // 23 = I2C1 reset
                                   // 25 = I2C2 reset

    volatile unsigned AHBENR;      // 0x14 - AHB Peripheral Clock Enable Register
                                   // Enables clocks for AHB peripherals.
                                   // Bits:
                                   // 0 = DMA1 clock enable
                                   // 1 = DMA2 clock enable
                                   // 2 = SRAM interface clock
                                   // 4 = FLITF clock
                                   // 8 = CRC clock

    volatile unsigned APB2ENR;     // 0x18 - APB2 Peripheral Clock Enable Register
                                   // Enables clocks for APB2 peripherals.
                                   // Bits:
                                   // 0 = AFIO clock enable
                                   // 2 = IOPA clock enable
                                   // 3 = IOPB clock enable
                                   // 4 = IOPC clock enable
                                   // 9 = ADC1 clock enable
                                   // 14 = USART1 clock enable

    volatile unsigned APB1ENR;     // 0x1C - APB1 Peripheral Clock Enable Register
                                   // Enables clocks for APB1 peripherals.
                                   // Bits:
                                   // 0  = TIM2 clock enable
                                   // 1  = TIM3 clock enable
                                   // 17 = USART2 clock enable
                                   // 18 = USART3 clock enable
                                   // 21 = SPI2 clock enable
                                   // 23 = I2C1 clock enable
                                   // 25 = I2C2 clock enable

    volatile unsigned BDCR;        // 0x20 - Backup Domain Control Register
                                   // Controls LSE oscillator and RTC.
                                   // Bits:
                                   // 0  = LSEON (enable 32.768 kHz crystal)
                                   // 1  = LSERDY (LSE ready)
                                   // 8  = RTCSEL (RTC clock source)
                                   // 15 = BDRST (backup domain reset)

    volatile unsigned CSR;         // 0x24 - Control/Status Register
                                   // Controls LSI and reset flags.
                                   // Bits:
                                   // 0  = LSION (enable internal 40 kHz RC)
                                   // 1  = LSIRDY (LSI ready)
                                   // 24 = RMVF (remove reset flags)
                                   // 26 = PINRSTF (pin reset flag)
                                   // 27 = PORRSTF (power-on reset flag)
                                   // 28 = SFTRSTF (software reset flag)
                                   // 29 = IWDGRSTF (independent watchdog reset)
                                   // 30 = WWDGRSTF (window watchdog reset)
                                   // 31 = LPWRRSTF (low-power reset)
};

/* Clock Sources */
enum clock_source {
    CLOCK_HSI = 0,      /* Internal 8MHz RC oscillator */
    CLOCK_HSE = 1,      /* External crystal oscillator */
    CLOCK_PLL = 2       /* Phase Locked Loop */
};

/* PLL Multiplication Factors */
enum pll_mult {
    PLL_MUL_2   = 0,    /* PLL clock = source * 2 */
    PLL_MUL_3   = 1,    /* PLL clock = source * 3 */
    PLL_MUL_4   = 2,    /* PLL clock = source * 4 */
    PLL_MUL_5   = 3,    /* PLL clock = source * 5 */
    PLL_MUL_6   = 4,    /* PLL clock = source * 6 */
    PLL_MUL_7   = 5,    /* PLL clock = source * 7 */
    PLL_MUL_8   = 6,    /* PLL clock = source * 8 */
    PLL_MUL_9   = 7,    /* PLL clock = source * 9 */
    PLL_MUL_10  = 8,    /* PLL clock = source * 10 */
    PLL_MUL_11  = 9,    /* PLL clock = source * 11 */
    PLL_MUL_12  = 10,   /* PLL clock = source * 12 */
    PLL_MUL_13  = 11,   /* PLL clock = source * 13 */
    PLL_MUL_14  = 12,   /* PLL clock = source * 14 */
    PLL_MUL_15  = 13,   /* PLL clock = source * 15 */
    PLL_MUL_16  = 14    /* PLL clock = source * 16 */
};

/* AHB Prescaler Values */
enum ahb_prescaler {
    AHB_DIV_1   = 0,    /* SYSCLK not divided */
    AHB_DIV_2   = 8,    /* SYSCLK divided by 2 */
    AHB_DIV_4   = 9,    /* SYSCLK divided by 4 */
    AHB_DIV_8   = 10,   /* SYSCLK divided by 8 */
    AHB_DIV_16  = 11,   /* SYSCLK divided by 16 */
    AHB_DIV_64  = 12,   /* SYSCLK divided by 64 */
    AHB_DIV_128 = 13,   /* SYSCLK divided by 128 */
    AHB_DIV_256 = 14,   /* SYSCLK divided by 256 */
    AHB_DIV_512 = 15    /* SYSCLK divided by 512 */
};

/* APB Prescaler Values */
enum apb_prescaler {
    APB_DIV_1   = 0,    /* HCLK not divided */
    APB_DIV_2   = 4,    /* HCLK divided by 2 */
    APB_DIV_4   = 5,    /* HCLK divided by 4 */
    APB_DIV_8   = 6,    /* HCLK divided by 8 */
    APB_DIV_16  = 7     /* HCLK divided by 16 */
};

/* USB Prescaler Values */
enum usb_prescaler {
    USB_DIV_1_5 = 0,    /* PLL clock divided by 1.5 */
    USB_DIV_1   = 1     /* PLL clock not divided */
};



void rcc_hsi_enable(void);
void rcc_hsi_disable(void);
void rcc_hse_enable(void);
void rcc_hse_disable(void);
void rcc_hse_bypass_enable(int enable);
void rcc_pll_enable(void);
void rcc_pll_disable(void);
void rcc_pll_config(unsigned source, unsigned multiplier);
void rcc_sysclk_config(unsigned source);
void rcc_ahb_prescaler_config(unsigned prescaler);
void rcc_apb1_prescaler_config(unsigned prescaler);
void rcc_apb2_prescaler_config(unsigned prescaler); 
void rcc_adc_prescaler_config(unsigned prescaler);
void rcc_usb_prescaler_config(unsigned prescaler); 
void rcc_mco_config(unsigned source);
void rcc_gpioa_enable(void);
void rcc_gpiob_enable(void);
void rcc_gpioc_enable(void);
void rcc_gpiod_enable(void);
void rcc_spi1_enable(void);
void rcc_spi2_enable(void);
void rcc_usart1_enable(void);
void rcc_usart2_enable(void);
void rcc_i2c1_enable(void);
void rcc_i2c2_enable(void);
void rcc_tim1_enable(void);
void rcc_tim2_enable(void);
void rcc_dma1_enable(void);
void rcc_afio_enable(void);
void rcc_adc1_enable(void);
void rcc_gpioa_reset(void);

void rcc_spi1_reset(void);
void rcc_sysclk_hsi_64mhz(void);
void rcc_sysclk_hse_72mhz(void);
void rcc_sysclk_default(void);
unsigned rcc_get_sysclk_frequency(void);
unsigned rcc_get_hclk_frequency(void);
unsigned rcc_get_pclk1_frequency(void);
void rcc_deinit(void);
void init_spi1_with_clocks(void);
void print_clock_frequencies(void);
