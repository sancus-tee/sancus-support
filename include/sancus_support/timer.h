#ifndef SANCUS_SUPPORT_TIMER_H
#define SANCUS_SUPPORT_TIMER_H
#include <msp430.h>

#define TACTL_DISABLE       (TACLR)
#define TACTL_ENABLE        (TASSEL_2 + MC_1 + TAIE)
#define TACTL_CONTINUOUS    ((TASSEL_2 + MC_2) & ~TAIE)

#if __GNUC__ >= 5
#define TIMER_IRQ_VECTOR    9 /* IRQ number 8 */
#else
#define TIMER_IRQ_VECTOR    16 /* IRQ number 8 */
#endif

void timer_disable(void);

/*
 * Fire an IRQ after the specified number of cycles have elapsed. Timer_A TAR
 * register will continue counting from zero after IRQ generation.
 */
void timer_irq(int interval);

/*
 * Operate Timer_A in continuous mode to act like a Time Stamp Counter.
 */
void timer_tsc_start(void);
int  timer_tsc_end(void);

/* Use for reactive OS support (sancus-support/src/main/main.c) */
void timer_init(void);

#endif
