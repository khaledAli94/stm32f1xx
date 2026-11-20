#ifndef SYS_CTRL_BLK_H
#define SYS_CTRL_BLK_H

#include <stdint.h>

#define SCB_BASE 0xe000ed00UL
#define SCB ((volatile struct scb_t *) SCB_BASE)

/* System control block (SCB) */
struct scb_t {
    volatile uint32_t CPUID;      // 0x00
    volatile uint32_t ICSR;       // 0x04
    volatile uint32_t VTOR;       // 0x08
    volatile uint32_t AIRCR;      // 0x0C
    volatile uint32_t SCR;        // 0x10
    volatile uint32_t CCR;        // 0x14
    volatile uint32_t SHPR1;      // 0x18
    volatile uint32_t SHPR2;      // 0x1C
    volatile uint32_t SHPR3;      // 0x20
    volatile uint32_t SHCSR;      // 0x24
    volatile uint32_t CFSR;       // 0x28
    volatile uint32_t HFSR;       // 0x2C
    uint32_t RESERVED0;
    volatile uint32_t MMAR;       // 0x34
    volatile uint32_t BFAR;       // 0x38
};

#endif /* SYS_CTRL_BLK_H */
