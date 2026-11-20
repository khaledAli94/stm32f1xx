#ifndef UART_H
#define UART_H

#include <stdint.h>

#define USART1_BASE 0x40013800UL
#define USART2_BASE 0x40004400UL
#define USART3_BASE 0x40004800UL

#define USART1 ((volatile struct usart_t *) USART1_BASE)
#define USART2 ((volatile struct usart_t *) USART2_BASE)
#define USART3 ((volatile struct usart_t *) USART3_BASE)

struct usart_t {
    volatile uint32_t SR;     // 0x00
    volatile uint32_t DR;     // 0x04
    volatile uint32_t BRR;    // 0x08
    volatile uint32_t CR1;    // 0x0C
    volatile uint32_t CR2;    // 0x10
    volatile uint32_t CR3;    // 0x14
    volatile uint32_t GTPR;   // 0x18
};

#endif /* UART_H */
