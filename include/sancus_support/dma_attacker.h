#ifndef SANCUS_SUPPORT_DMA_ATTACKER_H
#define SANCUS_SUPPORT_DMA_ATTACKER_H

#include <stdint.h>

#define DMA_ADDR      (*(volatile uint16_t**) 0x0070)
#define DMA_COUNTDOWN (*(volatile uint16_t*)  0x0072)
#define DMA_TRACE     (*(volatile uint16_t*)  0x0074)

#endif
