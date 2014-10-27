#ifndef SANCUS_SUPPORT_TSC_H
#define SANCUS_SUPPORT_TSC_H

#include <stdint.h>

#define TSC_VAL (*(volatile tsc_t*)  0x0190)
#define TSC_CTL (*(volatile uint8_t*)0x0190)

typedef uint64_t tsc_t;

static inline tsc_t tsc_read(void)
{
    // any write to TSC_CTL takes as snapshot of the current value which can be
    // read from TSC_VAL later
    TSC_CTL = 1;
    return TSC_VAL;
}

#endif
