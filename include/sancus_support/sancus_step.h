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

void sancus_step_print_latency(void);
int sancus_step_get_latency(void);

// sancus step interface
void sancus_step_start(void);
void sancus_step_init(void);
void sancus_step_end(void);

// sancus step configuration parameters
int ss_dbg_entry_delay;
int ss_dbg_measuring_reti_latency;
int isr_reti_latency;
int sm_exit_latency;
int isr_interrupted_sm;

extern struct SancusModule ssdbg;
int SM_ENTRY(ssdbg) ss_dbg_get_info(void);

volatile int      __ss_isr_tar_entry;

/*
    if (isr_interrupted_sm)
    {
        if (ss_dbg_measuring_reti_latency)
        {
            TACTL = TACTL_DISABLE;
            TACTL = TACTL_CONTINUOUS;
        }
        else
        {
            print_latency();
            timer_irq(isr_reti_latency);
        }
    }
    else
    {
        timer_disable();
    }
*/

#define SANCUS_STEP_ISR_ENTRY(fct)                                  \
__attribute__((naked)) __attribute__((interrupt(TIMER_IRQ_VECTOR))) \
void timerA_isr_entry(void)                                         \
{                                                                   \
    __asm__("mov &%0, &%2; save tar\n\t"                            \
            "mov #0x4, &%4; disable timer\n\t "                     \
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
            "; measuring isr_reti_latency\n\t"                      \
            "mov #0x220, &%4; set timer in continuous mode\n\t"     \
            "jmp 3f\n\t"                                            \
            "2: ; not measuring isr_reti_latency\n\t"               \
            "call #" #fct "\n\t"                                    \
            "push r15\n\t"                                          \
            "mov &%6, r15\n\t"                                      \
            "add #0x5, r15 ;\n\t"                                   \
            "mov r15, &%5\n\t"                                      \
            "pop r15\n\t"                                           \
            "mov #0x212, &%4; set timer in interrupt mode (irq)\n\t"\
            "jmp 3f\n\t"                                            \
            "1: ; sm not interrupted\n\t"                           \
            "call #timer_disable\n\t"                               \
            "3: reti\n\t"                                           \
            :                                                       \
            :                                                       \
            "m"(TAR),                                               \
            "m"(isr_interrupted_sm),                                \
            "m"(__ss_isr_tar_entry),                                \
            "m"(__isr_sp),                                          \
            "m"(TACTL),                                             \
            "m"(TACCR0),                                            \
            "m"(isr_reti_latency),                                  \
            "m"(ss_dbg_measuring_reti_latency)                      \
            :                                                       \
            );                                                      \
}

#endif
