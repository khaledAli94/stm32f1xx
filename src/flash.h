#define FLASH_BASE 0x40022000UL
#define FLASH ((volatile struct flash_t *) FLASH_BASE)

struct flash_t {
    volatile unsigned ACR;      // 0x00
    volatile unsigned KEYR;     // 0x04
    volatile unsigned OPTKEYR;  // 0x08
    volatile unsigned SR;       // 0x0C
    volatile unsigned CR;       // 0x10
    volatile unsigned AR;       // 0x14
    unsigned RESERVED0;
    volatile unsigned OBR;      // 0x1C
    volatile unsigned WRPR;     // 0x20
};

