#define USART1_BASE 0x40013800UL
#define USART2_BASE 0x40004400UL
#define USART3_BASE 0x40004800UL

#define USART1 ((volatile struct usart_t *) USART1_BASE)
#define USART2 ((volatile struct usart_t *) USART2_BASE)
#define USART3 ((volatile struct usart_t *) USART3_BASE)

struct usart_t {
    volatile unsigned SR;     // 0x00
    volatile unsigned DR;     // 0x04
    volatile unsigned BRR;    // 0x08
    volatile unsigned CR1;    // 0x0C
    volatile unsigned CR2;    // 0x10
    volatile unsigned CR3;    // 0x14
    volatile unsigned GTPR;   // 0x18
};

