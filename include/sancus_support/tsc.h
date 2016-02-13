#ifndef SANCUS_SUPPORT_TSC_H
#define SANCUS_SUPPORT_TSC_H

#include <stdint.h>

#define TSC_VAL (*(volatile tsc_t*)  0x0190)
#define TSC_CTL (*(volatile uint8_t*)0x0190)

typedef uint64_t tsc_t;

static inline __attribute__((always_inline)) tsc_t tsc_read(void)
{
    // any write to TSC_CTL takes as snapshot of the current value which can be
    // read from TSC_VAL later
    TSC_CTL = 1;
    return TSC_VAL;
}

#define DECLARE_TSC_TIMER(name)                                     \
    struct {tsc_t start, end;} name;                                \
    static void __attribute__((noinline)) name##_print_interval()   \
    {                                                               \
        printf(#name ": %llu\n", name.end - name.start - 28);       \
    }

#define TSC_TIMER_START(name)                               \
    do {                                                    \
        asm volatile("mov.b #1, &0x0190\n\t"                \
                     "mov.w &0x0190, &" #name "+0\n\t"      \
                     "mov.w &0x0192, &" #name "+2\n\t"      \
                     "mov.w &0x0194, &" #name "+4\n\t"      \
                     "mov.w &0x0196, &" #name "+6\n\t");    \
    } while (0)

#define TSC_TIMER_END(name)                                 \
    do {                                                    \
        asm volatile("mov.b #1, &0x0190\n\t"                \
                     "mov.w &0x0190, &" #name "+8\n\t"      \
                     "mov.w &0x0192, &" #name "+10\n\t"     \
                     "mov.w &0x0194, &" #name "+12\n\t"     \
                     "mov.w &0x0196, &" #name "+14\n\t");   \
        name##_print_interval();                            \
    } while (0)

#endif
