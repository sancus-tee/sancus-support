#ifndef SANCUS_SUPPORT_TSC_H
#define SANCUS_SUPPORT_TSC_H

#include <stdint.h>

#define TSC_BASE    0x190
#define TSC_SIZE    8

// any write to TSC_CTL takes as snapshot of the current value which can be
// read from TSC_VAL later
#define TSC_VAL (*(volatile tsc_t*)   TSC_BASE)
#define TSC_CTL (*(volatile uint8_t*) TSC_BASE)

typedef uint64_t tsc_t;

static inline __attribute__((always_inline)) tsc_t tsc_read(void)
{
    TSC_CTL = 1;
    return TSC_VAL;
}

#define DECLARE_TSC_TIMER(name)                                               \
    struct {tsc_t start, end;} name;                                          \
    static unsigned long long __attribute__((noinline)) name##_get_interval() \
    {                                                                         \
        return (name.end - name.start - 28);                                  \
    }                                                                         \
    static void __attribute__((noinline)) name##_print_interval()             \
    {                                                                         \
        printf(#name ": %llu\n", name##_get_interval());                      \
    }                                                                         \

#define TSC_TIMER_START(name)                               \
    do {                                                    \
        asm volatile("mov.b #1, %0\n\t"                     \
                     "mov.w %1+0, &" #name "+0\n\t"         \
                     "mov.w %1+2, &" #name "+2\n\t"         \
                     "mov.w %1+4, &" #name "+4\n\t"         \
                     "mov.w %1+6, &" #name "+6\n\t"         \
                     :: "m"(TSC_CTL), "m"(TSC_VAL):);       \
    } while (0)

#define TSC_TIMER_END(name)                                 \
    do {                                                    \
        asm volatile("mov.b #1, %0\n\t"                     \
                     "mov.w %1+0, &" #name "+8\n\t"         \
                     "mov.w %1+2, &" #name "+10\n\t"        \
                     "mov.w %1+4, &" #name "+12\n\t"        \
                     "mov.w %1+6, &" #name "+14\n\t"        \
                     :: "m"(TSC_CTL), "m"(TSC_VAL):);       \
    } while (0)

#endif
