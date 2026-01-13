#include <gpio.h>

/* GPIO Driver Functions */
volatile struct gpio_t *gpio_get_port(enum gpio_port port) {
    switch (port) {
        case PORT_A: return GPIOA;
        case PORT_B: return GPIOB;
        case PORT_C: return GPIOC;
        case PORT_D: return GPIOD;
    }
    return GPIOA; /* Default to GPIOA */
}

void gpio_pin_mode(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_mode mode) {
    unsigned config_reg;
    unsigned pin_pos;
    unsigned clear_mask;
    
    /* Determine which configuration register to use - access gpio->CRL (offset 0x00) or gpio->CRH (offset 0x04) */
    if (pin < 8) {
        config_reg = gpio->CRL;  /* Access CRL register (offset 0x00) */
        pin_pos = pin * 4;       /* Each pin uses 4 bits in CRL */
    } else {
        config_reg = gpio->CRH;  /* Access CRH register (offset 0x04) */
        pin_pos = (pin - 8) * 4; /* Each pin uses 4 bits in CRH */
    }
    
    /* Clear the 4-bit configuration for this pin */
    clear_mask = ~(0xF << pin_pos);
    config_reg &= clear_mask;
    
    /* Set the new mode */
    config_reg |= (mode << pin_pos);
    
    /* Write back to the appropriate register */
    if (pin < 8) {
        gpio->CRL = config_reg;  /* Write to CRL register (offset 0x00) */
    } else {
        gpio->CRH = config_reg;  /* Write to CRH register (offset 0x04) */
    }
}

void gpio_pin_write(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_state state) {
    if (state == GPIO_HIGH) {
        /* Set pin using BSRR - access gpio->BSRR (offset 0x10) */
        gpio->BSRR = (1 << pin);  /* Set bit (BSR[pin]) */
    } else {
        /* Reset pin using BSRR - access gpio->BSRR (offset 0x10) */
        gpio->BSRR = (1 << (pin + 16));  /* Reset bit (BR[pin]) */
    }
}

void gpio_pin_toggle(volatile struct gpio_t *gpio, enum gpio_pin pin) {
    /* Read current state from ODR - access gpio->ODR (offset 0x0C) */
    unsigned current_state = (gpio->ODR >> pin) & 0x1;
    
    if (current_state) {
        /* If high, set it low using BRR - access gpio->BRR (offset 0x14) */
        gpio->BRR = (1 << pin);  /* Reset pin */
    } else {
        /* If low, set it high using BSRR - access gpio->BSRR (offset 0x10) */
        gpio->BSRR = (1 << pin);  /* Set pin */
    }
}

enum gpio_state gpio_pin_read(volatile struct gpio_t *gpio, enum gpio_pin pin) {
    /* Read pin state from IDR - access gpio->IDR (offset 0x08) */
    unsigned pin_state = (gpio->IDR >> pin) & 0x1;
    
    return (pin_state) ? GPIO_HIGH : GPIO_LOW;
}

void gpio_port_write(volatile struct gpio_t *gpio, unsigned short value) {
    /* Write to all pins at once - access gpio->ODR (offset 0x0C) */
    gpio->ODR = value & 0xFFFF;  /* Only 16 pins, mask to 16 bits */
}

unsigned short gpio_port_read(volatile struct gpio_t *gpio) {
    /* Read all pins at once - access gpio->IDR (offset 0x08) */
    return (unsigned short)(gpio->IDR & 0xFFFF);  /* Only 16 pins */
}

void gpio_pin_set_high(volatile struct gpio_t *gpio, enum gpio_pin pin) {
    /* Set pin high using BSRR - access gpio->BSRR (offset 0x10) */
    gpio->BSRR = (1 << pin);  /* Set bit (BSR[pin]) */
}

void gpio_pin_set_low(volatile struct gpio_t *gpio, enum gpio_pin pin) {
    /* Set pin low using BRR - access gpio->BRR (offset 0x14) */
    gpio->BRR = (1 << pin);  /* Reset bit */
}

void gpio_set_pull(volatile struct gpio_t *gpio, enum gpio_pin pin, enum gpio_pull pull) {
    /* For pull-up/pull-down, we need to:
       1. Set pin as input with pull-up/pull-down mode
       2. Set ODR bit to determine pull direction */
    
    /* First set as input with pull-up/pull-down mode */
    gpio_pin_mode(gpio, pin, GPIO_INPUT_PU_PD);
    
    /* Then set ODR bit for pull-up or pull-down */
    if (pull == GPIO_PULL_UP) {
        /* Set ODR bit for pull-up - access gpio->ODR (offset 0x0C) */
        gpio->ODR |= (1 << pin);  /* Pull-up: ODR bit = 1 */
    } else {
        /* Clear ODR bit for pull-down - access gpio->ODR (offset 0x0C) */
        gpio->ODR &= ~(1 << pin);  /* Pull-down: ODR bit = 0 */
    }
}

void gpio_config_output(volatile struct gpio_t *gpio, enum gpio_pin pin, int speed) {
    enum gpio_mode mode;
    
    /* Determine mode based on speed */
    switch (speed) {
        case 2:
            mode = GPIO_OUTPUT_PP_2MHZ;
            break;
        case 50:
            mode = GPIO_OUTPUT_PP_50MHZ;
            break;
        case 10:
        default:
            mode = GPIO_OUTPUT_PP_10MHZ;
            break;
    }
    
    gpio_pin_mode(gpio, pin, mode);
}

void gpio_config_input(volatile struct gpio_t *gpio, enum gpio_pin pin, int with_pull) {
    enum gpio_mode mode;
    
    if (with_pull) {
        mode = GPIO_INPUT_PU_PD;
    } else {
        mode = GPIO_INPUT_FLOATING;
    }
    
    gpio_pin_mode(gpio, pin, mode);
}

void gpio_config_alternate(volatile struct gpio_t *gpio, enum gpio_pin pin, int speed, int open_drain) {
    enum gpio_mode mode;
    
    /* Determine mode based on speed and open-drain setting */
    if (open_drain) {
        switch (speed) {
            case 2:
                mode = GPIO_AF_OD_2MHZ;
                break;
            case 50:
                mode = GPIO_AF_OD_50MHZ;
                break;
            case 10:
            default:
                mode = GPIO_AF_OD_10MHZ;
                break;
        }
    } else {
        switch (speed) {
            case 2:
                mode = GPIO_AF_PP_2MHZ;
                break;
            case 50:
                mode = GPIO_AF_PP_50MHZ;
                break;
            case 10:
            default:
                mode = GPIO_AF_PP_10MHZ;
                break;
        }
    }
    
    gpio_pin_mode(gpio, pin, mode);
}