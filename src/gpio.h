#define GPIOA_BASE 0x40010800UL
#define GPIOB_BASE 0x40010C00UL
#define GPIOC_BASE 0x40011000UL
#define GPIOD_BASE 0x40011400UL

#define GPIOA ((volatile struct gpio_t *) GPIOA_BASE)
#define GPIOB ((volatile struct gpio_t *) GPIOB_BASE)
#define GPIOC ((volatile struct gpio_t *) GPIOC_BASE)
#define GPIOD ((volatile struct gpio_t *) GPIOD_BASE)

struct gpio_t{
    volatile unsigned CRL;     // 0x00
    volatile unsigned CRH;     // 0x04
    volatile unsigned IDR;     // 0x08
    volatile unsigned ODR;     // 0x0C
    volatile unsigned BSRR;    // 0x10
    volatile unsigned BRR;     // 0x14
    volatile unsigned LCKR;    // 0x18
};


/* GPIO Pin States */
enum gpio_state {
    GPIO_LOW = 0,
    GPIO_HIGH = 1
};

/* GPIO Pull-up/Pull-down */
enum gpio_pull {
    GPIO_PULL_DOWN = 0,
    GPIO_PULL_UP   = 1
};

/* GPIO Mode Configuration */
enum gpio_mode {
    GPIO_INPUT_ANALOG      = 0,   /* 0b0000 */
    GPIO_INPUT_FLOATING    = 4,   /* 0b0100 */
    GPIO_INPUT_PU_PD       = 8,   /* 0b1000 */

    GPIO_OUTPUT_PP_10MHZ   = 1,   /* 0b0001 */
    GPIO_OUTPUT_PP_2MHZ    = 2,   /* 0b0010 */
    GPIO_OUTPUT_PP_50MHZ   = 3,   /* 0b0011 */

    GPIO_OUTPUT_OD_10MHZ   = 5,   /* 0b0101 */
    GPIO_OUTPUT_OD_2MHZ    = 6,   /* 0b0110 */
    GPIO_OUTPUT_OD_50MHZ   = 7,   /* 0b0111 */

    GPIO_AF_PP_10MHZ       = 9,   /* 0b1001 */
    GPIO_AF_PP_2MHZ        = 10,  /* 0b1010 */
    GPIO_AF_PP_50MHZ       = 11,  /* 0b1011 */

    GPIO_AF_OD_10MHZ       = 13,  /* 0b1101 */
    GPIO_AF_OD_2MHZ        = 14,  /* 0b1110 */
    GPIO_AF_OD_50MHZ       = 15   /* 0b1111 */
};

/* GPIO Pin Numbers */
enum gpio_pin {
    PIN_0 = 0,
    PIN_1 = 1,
    PIN_2 = 2,
    PIN_3 = 3,
    PIN_4 = 4,
    PIN_5 = 5,
    PIN_6 = 6,
    PIN_7 = 7,
    PIN_8 = 8,
    PIN_9 = 9,
    PIN_10 = 10,
    PIN_11 = 11,
    PIN_12 = 12,
    PIN_13 = 13,
    PIN_14 = 14,
    PIN_15 = 15
};

/* GPIO Ports */
enum gpio_port {
    PORT_A,
    PORT_B,
    PORT_C,
    PORT_D
};

/* GPIO Driver Functions */
volatile struct gpio_t *gpio_get_port(enum gpio_port port);
void gpio_pin_mode(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_mode mode);
void gpio_pin_write(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_state state);
void gpio_pin_toggle(volatile struct gpio_t *gpio, enum gpio_pin pin);
enum gpio_state gpio_pin_read(volatile struct gpio_t *gpio, enum gpio_pin pin);
void gpio_port_write(volatile struct gpio_t *gpio, unsigned short value);
unsigned short gpio_port_read(volatile struct gpio_t *gpio);
void gpio_pin_set_high(volatile struct gpio_t *gpio, enum gpio_pin pin);
void gpio_pin_set_low(volatile struct gpio_t *gpio, enum gpio_pin pin);
void gpio_set_pull(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_pull pull);
void gpio_config_output(volatile struct gpio_t *gpio, enum gpio_pin pin, int speed);
void gpio_config_input(volatile struct gpio_t *gpio, enum gpio_pin pin, int with_pull);
void gpio_config_alternate(volatile struct gpio_t *gpio, enum gpio_pin pin, int speed, int open_drain);