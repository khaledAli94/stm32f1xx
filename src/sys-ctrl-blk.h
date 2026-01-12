struct scb_t {
    volatile unsigned CPUID;      // 0x00
    volatile unsigned ICSR;       // 0x04
    volatile unsigned VTOR;       // 0x08
    volatile unsigned AIRCR;      // 0x0C
    volatile unsigned SCR;        // 0x10
    volatile unsigned CCR;        // 0x14
    volatile unsigned SHPR1;      // 0x18
    volatile unsigned SHPR2;      // 0x1C
    volatile unsigned SHPR3;      // 0x20
    volatile unsigned SHCSR;      // 0x24
    volatile unsigned CFSR;       // 0x28
    volatile unsigned HFSR;       // 0x2C
    unsigned RESERVED0;
    volatile unsigned MMAR;       // 0x34
    volatile unsigned BFAR;       // 0x38
};

#define SCB_BASE 0xe000ed00UL
#define SCB ((volatile struct scb_t *) SCB_BASE)
