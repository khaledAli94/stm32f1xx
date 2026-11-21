#ifndef DMA_H
#define DMA_H

#include <stdint.h>

#define DMA1_BASE 0x40020000UL
#define DMA2_BASE 0x40020400UL

#define DMA1 ((volatile struct dma_t *) DMA1_BASE) // 7 channel 
#define DMA2 ((volatile struct dma_t *) DMA2_BASE) // 5 channel

struct dma_channel_t {
    volatile uint32_t CCR;     // 0x00 cfg
    volatile uint32_t CNDTR;   // 0x04 cntr
    volatile uint32_t CPAR;    // 0x08 periph addr
    volatile uint32_t CMAR;    // 0x0C mem addr
    uint32_t RESERVED0;
};

struct dma_t {
    volatile uint32_t ISR;     // 0x00
    volatile uint32_t IFCR;    // 0x04 
    struct dma_channel_t CH[7];
};

#endif /* DMA_H */
