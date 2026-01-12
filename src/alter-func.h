#define AFIO_BASE 0x40010000UL
#define AFIO ((volatile struct afio_t *) AFIO_BASE)

struct afio_t {
    volatile unsigned EVCR;        // 0x00
    volatile unsigned MAPR;        // 0x04
    volatile unsigned EXTICR1;     // 0x08
    volatile unsigned EXTICR2;     // 0x0C
    volatile unsigned EXTICR3;     // 0x10
    volatile unsigned EXTICR4;     // 0x14
    volatile unsigned MAPR2;       // 0x18
};
