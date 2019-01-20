#ifndef SANCUS_STEP_H
#define SANCUS_STEP_H
#include <msp430.h>
#include "timer.h"
#include <stdint.h>

// note the difference between the decimal and the hexadecimal constants
#define HW_IRQ_LATENCY          34
#define ISR_STACK_SIZE          512
#define INIT_LATENCY            42

#define SANCUS_STEP_ISR(fct) \
    SANCUS_STEP_ISR_INTERNAL(fct, 0x42, 0x212)

void print_latency(void);
int get_latency(void);

uint16_t __ss_isr_stack[ISR_STACK_SIZE];
void*    __ss_isr_sp;
void*    __ss_isr_reti_addr;
volatile int      __ss_isr_tar_entry;

/*
 * The first latency will be useless, but this is ok for now.
 * We can fix this by adjusting the INIT_LATENCY definition.
 */
#define SANCUS_STEP_INIT                    \
    __asm__("dint\n\t");                    \
    TACTL = TACTL_DISABLE;                  \
    TACCR0 = INIT_LATENCY;                  \
    /* source mclk, up mode */              \
    TACTL = TACTL_ENABLE;

/*
 * Execute fct after every instruction of the sancus module
 */
#define SANCUS_STEP_ISR_INTERNAL(fct, sm_reti_val, ta_enable)   \
    __asm__("mov &%0, &__ss_isr_tar_entry               \n\t"   \
            "cmp #0x0, r1                               \n\t"   \
            "jne 1f                                     \n\t"   \
            "; sm got interrupted                       \n\t"   \
            "mov &__ss_isr_sp, r1                       \n\t"   \
            "push r15                                   \n\t"   \
            "push #0x0                                  \n\t"   \
            "call #" #fct "                             \n\t"   \
            "mov #" #sm_reti_val ", &%1                 \n\t"   \
            "mov #" #ta_enable ", &%2                   \n\t"   \
            "jmp 2f                                     \n\t"   \
            "1:                                         \n\t"   \
            "; no sm interrupted                        \n\t"   \
            "mov #0x4, &%2 ;disable timerA              \n\t"   \
            "2: ; cont after if-then-else               \n\t"   \
            "reti                                       \n\t"   \
            ::"m"(TAR),"m"(TACCR0),"m"(TACTL):);


#endif
