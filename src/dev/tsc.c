#include "tsc.h"

#define TSC (*(volatile tsc_t*)0x0190)

tsc_t tsc_read(void)
{
    // any write to TSC takes as snapshot of the current value which can be read
    // later
    TSC = 1;
    return TSC;
}
