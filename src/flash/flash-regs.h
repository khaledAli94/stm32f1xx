#ifndef FLASH_REGS_H
#define FLASH_REGS_H

#define FLASH_REG_BASE 0x40022000UL
#define FLASH ((volatile struct flash_regs_t *) FLASH_REG_BASE)

struct flash_regs_t {
    volatile unsigned ACR;      // 0x00 - Access Control Register
    volatile unsigned KEYR;     // 0x04 - Key Register
    volatile unsigned OPTKEYR;  // 0x08 - Option Key Register
    volatile unsigned SR;       // 0x0C - Status Register
    volatile unsigned CR;       // 0x10 - Control Register
    volatile unsigned AR;       // 0x14 - Address Register
    unsigned RESERVED0;         // 0x18 - Reserved
    volatile unsigned OBR;      // 0x1C - Option Byte Register
    volatile unsigned WRPR;     // 0x20 - Write Protection Register
};

static inline void flash_set_latency(unsigned freq_hz) {
    unsigned acr = FLASH->ACR;

    /* Clear LATENCY[2:0] bits (bits 2:0) and PRFTBE (bit 4) */
    acr &= ~0x1fU;

    if (freq_hz <= 24000000) {
        acr |= 0x0U;  /* 0 wait states */
    } else if (freq_hz <= 48000000) {
        acr |= (1U << 4) | 0x1U;  /* 1 wait state + prefetch */
    } else {
        acr |= (1U << 4) | 0x2U;  /* 2 wait states + prefetch */
    }

    FLASH->ACR = acr;
}

#endif /* FLASH_REGS_H */
