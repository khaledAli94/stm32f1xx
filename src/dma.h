#define DMA1_BASE 0x40020000UL
#define DMA2_BASE 0x40020400UL

#define DMA1 ((volatile struct dma_t *) DMA1_BASE)
#define DMA2 ((volatile struct dma_t *) DMA2_BASE)

struct dma_channel_t {
    volatile unsigned CCR;     // 0x00
    volatile unsigned CNDTR;   // 0x04
    volatile unsigned CPAR;    // 0x08
    volatile unsigned CMAR;    // 0x0C
    unsigned RESERVED0;
};

struct dma_t {
    volatile unsigned ISR;     // 0x00
    volatile unsigned IFCR;    // 0x04
    struct dma_channel_t CH[7];
};

