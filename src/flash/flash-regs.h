#ifndef FLASH_REGS_H
#define FLASH_REGS_H

#include <stdint.h>

#define FLASH_REG_BASE 0x40022000UL
#define FLASH ((volatile struct flash_regs_t *) FLASH_REG_BASE)

struct flash_regs_t {
    volatile uint32_t ACR;      // 0x00 - Access Control Register
    volatile uint32_t KEYR;     // 0x04 - Key Register
    volatile uint32_t OPTKEYR;  // 0x08 - Option Key Register
    volatile uint32_t SR;       // 0x0C - Status Register
    volatile uint32_t CR;       // 0x10 - Control Register
    volatile uint32_t AR;       // 0x14 - Address Register
    uint32_t RESERVED0;         // 0x18 - Reserved
    volatile uint32_t OBR;      // 0x1C - Option Byte Register
    volatile uint32_t WRPR;     // 0x20 - Write Protection Register
};

static inline void flash_set_latency(uint32_t freq_hz) {
    uint32_t acr = FLASH->ACR;

    /* Clear LATENCY[2:0] bits (bits 2:0) and PRFTBE (bit 4) */
    acr &= ~0x1fU;

    if (freq_hz <= 24000000U) {
        acr |= 0x0U;  /* 0 wait states */
    } else if (freq_hz <= 48000000U) {
        acr |= (1U << 4) | 0x1U;  /* 1 wait state + prefetch */
    } else {
        acr |= (1U << 4) | 0x2U;  /* 2 wait states + prefetch */
    }

    FLASH->ACR = acr;
}

#endif /* FLASH_REGS_H */
