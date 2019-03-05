#ifndef SANCUS_SUPPORT_TIMER_H
#define SANCUS_SUPPORT_TIMER_H
#include <msp430.h>
#include <stdint.h>

#define TACTL_DISABLE       (TACLR) // 0x04
#define TACTL_ENABLE        (TASSEL_2 + MC_1 + TAIE) // 0x212
#define TACTL_CONTINUOUS    ((TASSEL_2 + MC_2) & ~TAIE) // 0x220

#define TIMER_IRQ_VECTOR    16 /* IRQ number 8 */

#define ISR_STACK_SIZE (512)

uint16_t __isr_stack[ISR_STACK_SIZE];
extern void* __isr_sp;

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


#define TIMER_ISR_ENTRY(fct)                                        \
__attribute__((naked)) __attribute__((interrupt(TIMER_IRQ_VECTOR))) \
void timerA_isr_entry(void)                                         \
{                                                                   \
    __asm__ __volatile__(                                           \
            "cmp #0x0, r1\n\t"                                      \
            "jne 1f\n\t"                                            \
            "mov &__isr_sp, r1\n\t"                                 \
            "push r15\n\t"                                          \
            "push #0x0\n\t"                                         \
            "1: call #" #fct "\n\t"                                 \
            "reti\n\t"                                              \
            :::);                                                   \
}


#endif
