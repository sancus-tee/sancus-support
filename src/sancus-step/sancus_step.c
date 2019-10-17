#include "sancus_step.h"
#include "timer.h"
#include <stdio.h>
#include <sancus/sm_support.h>
#include "sm_io.h"

void __ss_print_latency(void)
{
    int latency = __ss_get_latency();
    printf("latency: %d\n", latency);
}

int __ss_get_latency(void)
{
    return __ss_isr_tar_entry - __ss_sm_exit_latency;
}

int __ss_dbg_entry_delay = 0;
int __ss_dbg_measuring_reti_latency = 0;
int __ss_dbg_init_done = 0;
int __ss_isr_reti_latency = 0;
int __ss_sm_exit_latency = 0;
int __ss_isr_interrupted_sm = 0;

/*
 * Determines isr_reti_latency (reti into interrupted moduled)
 * and sm_exit_latency (from interrupt to isr)
 */
void __ss_init(void)
{
    sancus_enable(&ssdbg);

    __ss_dbg_measuring_reti_latency = 1;

    timer_tsc_start();
    __ss_dbg_get_info();

    timer_irqc(__ss_dbg_entry_delay);
    int isr_reti_latency_t = __ss_dbg_get_info();
    /* amount of cycles in the reti logic = measured delay
                                  - record delay
                                  - duration of reti instruction
                                  + 1 (because we count amount of cycles)
    */
    __ss_isr_reti_latency = isr_reti_latency_t
                        - JMP_LENGTH
                        - RETI_LENGTH
                        + 1;

    timer_irqc(__ss_dbg_entry_delay + EXTRA_DELAY);
    __ss_dbg_get_info();
    __ss_sm_exit_latency = __ss_isr_tar_entry - ENTRY_DELAY;
    printf("resume latency: %d\n", __ss_isr_reti_latency);
    printf("exit latency: %d\n", __ss_sm_exit_latency);
    printf("entry delay: %d\n", __ss_dbg_entry_delay);

    __ss_dbg_measuring_reti_latency = 0;
    __ss_dbg_init_done = 1;
}

/*
 * Determines isr_reti_latency (reti into interrupted module)
 * and sm_exit_latency (from interrupt to isr)
 * and starts timer, so that nemesis can be executed
 */
void __ss_start(void)
{
    if (__ss_dbg_init_done == 0) {
        __ss_init();
    }
    __asm__("dint\n\t"); // disable interrupts
    timer_irqc(INIT_LATENCY);
}

void __ss_end(void)
{
    TACTL = TACTL_DISABLE;
}

DECLARE_SM(ssdbg, 0x1234);

__attribute__((naked))
int SM_ENTRY(ssdbg) __ss_dbg_get_info(void)
{
    __asm__ __volatile__(
        /* __ss_dbg_entry_delay = TAR */
        "mov &%0, &__ss_dbg_entry_delay\n\t"
        /* ------------------> 1st IRQ should arrive here <------------------ */
        /* r15 = TAR */
        "mov &%0, r15\n\t"
        /* TACTL = TACTL_DISABLE */
        "mov %3, &%1\n\t"
        /* TACTL = TACTL_CONTINUOUS */
        "mov %2, &%1\n\t"
        /* ------------------> 2nd IRQ should arrive here <------------------ */
        /* r15 = r15 - 1 */
        "sub #0x1, r15\n\t"
        /* return from subroutine */
        "ret\n\t"
        :
        :
        "m"(TAR),               // %0
        "m"(TACTL),             // %1
        "i"(TACTL_CONTINUOUS),  // %2
        "i"(TACTL_DISABLE)      // %3
        :
    );
}
