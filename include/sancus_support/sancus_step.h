#ifndef SANCUS_STEP_H
#define SANCUS_STEP_H
#include <msp430.h>
#include "timer.h"
#include <stdint.h>
#include <sancus/sm_support.h>


/* ======== SANCUS STEP DBG CONSTANTS ======== */
#define RETI_LENGTH (0x5)
#define JMP_LENGTH (0x2)

#define EXTRA_DELAY (0x2)
#define ENTRY_DELAY (0x3)

#define INIT_LATENCY 1

void __ss_print_latency(void);
int __ss_get_latency(void);

// sancus step interface
void __ss_start(void);
void __ss_init(void);
void __ss_end(void);

// sancus step configuration parameters
int __ss_dbg_entry_delay;
int __ss_dbg_measuring_reti_latency;
int __ss_isr_reti_latency;
int __ss_sm_exit_latency;
int __ss_isr_interrupted_sm;

extern struct SancusModule ssdbg;
int SM_ENTRY(ssdbg) __ss_dbg_get_info(void);

volatile int      __ss_isr_tar_entry;

#define SANCUS_STEP_ISR_ENTRY2(fct_single_step, fct_end)            \
__attribute__((naked)) __attribute__((interrupt(TIMER_IRQ_VECTOR2)))\
void timerA_isr_entry2(void)                                        \
{                                                                   \
    __asm__("mov &%0, &%2; save tar\n\t"                            \
            "mov %9, &%4; disable timer\n\t"                        \
            "sub &%5, &%2; subtract TACCR0 from timestamp\n\t"      \
            "mov #0x0, &%1\n\t"                                     \
            "cmp #0x0, r1\n\t"                                      \
            "jne 1f\n\t"                                            \
            "; sm got interrupted\n\t"                              \
            "mov #0x1, &%1\n\t"                                     \
            "mov &%3, r1\n\t"                                       \
            "push r15\n\t"                                          \
            "push #0x0\n\t"                                         \
            "cmp #0x0, &%7\n\t"                                     \
            "jz 2f\n\t"                                             \
            "; measuring __ss_isr_reti_latency\n\t"                 \
            "mov %8, &%4; set timer in continuous mode\n\t"         \
            "jmp 3f\n\t"                                            \
            "2: ; not measuring __ss_isr_reti_latency\n\t"          \
            "call #" #fct_single_step "\n\t"                        \
            "push r15\n\t"                                          \
            "mov &%6, r15\n\t"                                      \
            "add #0x6, r15 ;\n\t"                                   \
            "mov r15, &%5\n\t"                                      \
            "pop r15\n\t"                                           \
            "mov %11, &%12; set timer in interrupt mode (irqc)\n\t" \
            "mov %13, &%4; set timer in interrupt mode 2 (irqc)\n\t"\
            "jmp 3f\n\t"                                            \
            "1: ; sm not interrupted\n\t"                           \
            "mov %9, &%4; disable timer\n\t"                        \
            "call #" #fct_end "\n\t"                                \
            "3: reti\n\t"                                           \
            :                                                       \
            :                                                       \
            "m"(TAR),                                  /* %0 */     \
            "m"(__ss_isr_interrupted_sm),              /* %1 */     \
            "m"(__ss_isr_tar_entry),                   /* %2 */     \
            "m"(__isr_sp),                             /* %3 */     \
            "m"(TACTL),                                /* %4 */     \
            "m"(TACCR0),                               /* %5 */     \
            "m"(__ss_isr_reti_latency),                /* %6 */     \
            "m"(__ss_dbg_measuring_reti_latency),      /* %7 */     \
            "i"(TACTL_CONTINUOUS),                     /* %8 */     \
            "i"(TACTL_DISABLE),                        /* %9 */     \
            "i"(TACTL_ENABLE),                         /* %10 */    \
            "i"(TACCTL_ENABLE_CONT),                   /* %11 */    \
            "m"(TACCTL0),                              /* %12 */    \
            "i"(TACTL_ENABLE_CONT)                     /* %13 */    \
            :                                                       \
            );                                                      \
}

#endif
