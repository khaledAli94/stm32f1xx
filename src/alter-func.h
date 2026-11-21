#ifndef ALTER_FUNC_H
#define ALTER_FUNC_H

#include <stdint.h>

#define AFIO_BASE 0x40010000UL
#define AFIO ((volatile struct afio_t *) AFIO_BASE)

struct afio_t {
    volatile uint32_t EVCR;        // 0x00
    volatile uint32_t MAPR;        // 0x04
    volatile uint32_t EXTICR1;     // 0x08
    volatile uint32_t EXTICR2;     // 0x0C
    volatile uint32_t EXTICR3;     // 0x10
    volatile uint32_t EXTICR4;     // 0x14
    volatile uint32_t MAPR2;       // 0x18
};

#endif /* ALTER_FUNC_H */
