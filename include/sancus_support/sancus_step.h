#ifndef SANCUS_STEP_H
#define SANCUS_STEP_H
#include <msp430.h>
#include "timer.h"
#include <stdint.h>
#include <sancus/sm_support.h>


/* ======== SANCUS STEP DBG CONSTANTS ======== */
#define RETI_LENGTH (0x5)
#define JMP_LENGTH (0x2)
#define RET_LENGTH (0x3)

#define EXTRA_DELAY (0x2)

#define INIT_LATENCY 1

void print_latency(void);
int get_latency(void);

// new interface
void sancus_step_start(void);
void sancus_step_init(void);
void sancus_step_end(void);
void sancus_step_isr_main(void);

int ss_dbg_entry_delay;
int ss_dbg_measuring_reti_latency;
int isr_reti_latency;
int sm_exit_latency;
int isr_interrupted_sm;

extern struct SancusModule ssdbg;
int SM_ENTRY(ssdbg) ss_dbg_get_info(void);

volatile int      __ss_isr_tar_entry;

#define SANCUS_STEP_ISR_ENTRY(fct)                                  \
__attribute__((naked)) __attribute__((interrupt(TIMER_IRQ_VECTOR))) \
void timerA_isr_entry(void)                                         \
{                                                                   \
    __asm__("mov &%0, &__ss_isr_tar_entry\n\t"                      \
            "cmp #0x0, r1\n\t"                                      \
            "jne 1f\n\t"                                            \
            "; sm got interrupted\n\t"                              \
            "mov #0x1, &isr_interrupted_sm\n\t"                     \
            "jmp cont\n\t"                                          \
            "1: mov #0x0, &isr_interrupted_sm\n\t"                  \
            "cont:\n\t"                                             \
            "mov &__isr_sp, r1\n\t"                                 \
            "push r15\n\t"                                          \
            "push #0x0\n\t"                                         \
            "call #" #fct "\n\t"                                    \
            "reti\n\t"                                              \
            ::"m"(TAR):);                                           \
}

#endif
