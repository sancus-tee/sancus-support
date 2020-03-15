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

extern int __ss_dbg_entry_delay = 0;
extern int __ss_dbg_measuring_reti_latency = 0;
extern int __ss_isr_reti_latency = 0;
extern int __ss_sm_exit_latency = 0;
extern int __ss_isr_interrupted_sm = 0;

volatile int __ss_isr_tar_entry;

/*
 * Determines isr_reti_latency (reti into interrupted module)
 * and sm_exit_latency (from interrupt to isr)
 * and starts timer, so that nemesis can be executed
 */
void __ss_start(void)
{
    __ss_init();
    __ss_mount(NULL);
}

void __ss_mount(const char * trace)
{
    if (trace != NULL)
      printf("ss_mount: %s\n", trace);

    __asm__("dint\n\t");
    timer_irqc(INIT_LATENCY);
}

/*
 * Determines isr_reti_latency (reti into interrupted moduled)
 * and sm_exit_latency (from interrupt to isr)
 */
void __ss_init(void)
{
    sancus_enable(&sancus_step);
    
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

#if __clang_major__ >= 5
    // TODO: The measurement of the exit latency as implemented by
    //       Sven Cuyt does not seem to work. For now, overwrite the 
    //       measurement with a hardcoded value.
    __ss_sm_exit_latency = 35 /* cycles */;
#endif
}

void __ss_end(void)
{
    TACTL = TACTL_DISABLE;
}

DECLARE_SM(sancus_step, 0x1234);

__attribute__((naked))
int SM_ENTRY(sancus_step) __ss_dbg_get_info(void)
{
    
    __asm__ __volatile__(
                "mov &%0, &__ss_dbg_entry_delay; 1st irq should arrive here\n\t"
#if __clang_major__ >= 5
                "mov &%0, r12\n\t"
#else
                "mov &%0, r15\n\t"
#endif
                "mov %3, &%1\n\t"
                "mov %2, &%1; 2nd irq should arrive here\n\t"
#if __clang_major__ >= 5
                "sub #0x1, r12\n\t"
#else
                "sub #0x1, r15\n\t"
#endif
                "ret\n\t"
                ::
                "m"(TAR), "m"(TACTL),
                "i"(TACTL_CONTINUOUS), "i"(TACTL_DISABLE):
                );
}
