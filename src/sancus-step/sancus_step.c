#include "sancus_step.h"
#include "timer.h"
#include <stdio.h>
#include <sancus/sm_support.h>

void* __ss_isr_sp = &__ss_isr_stack[ISR_STACK_SIZE-1];

void print_latency(void)
{
    TACTL = TACTL_DISABLE;
    int latency = __ss_isr_tar_entry - HW_IRQ_LATENCY - 1;
    printf("latency: %d\n", latency);
}

int get_latency(void)
{
    TACTL = TACTL_DISABLE;
    return __ss_isr_tar_entry - HW_IRQ_LATENCY - 1;
}

int ss_dbg_entry_delay = 0;
int ss_dbg_interrupted = 0;
int isr_latency = 0;

/*
 * Determines isr_latency (reti into interrupted module)
 * and starts timer, so that nemesis can be executed
 */
void sancus_step_start(void)
{
    sancus_step_init();
    __asm__("dint\n\t");
    timer_irq(INIT_LATENCY);
}

/*
 * Determines isr_latency (reti into interrupted moduled)
 */
void sancus_step_init(void)
{
    ss_dbg_interrupted = 0;
    timer_tsc_start();
    int isr_latency_t = ss_dbg_get_isr_latency();
    timer_irq(ss_dbg_entry_delay);
    isr_latency_t = ss_dbg_get_isr_latency();
    /* amount of cycles in the reti logic = measured delay
                                  - record delay
                                  - duration of reti instruction
                                  + 1 (because we count amount of cycles)
    */
    isr_latency = isr_latency_t - SS_DBG_RECORD_DELAY - RETI_LENGTH + 1;
}

void sancus_step_end(void)
{
    TACTL = TACTL_DISABLE;
}

void sancus_step_isr_main(void)
{
    
}

DECLARE_SM(ssdbg, 0x1234);

int SM_ENTRY(ssdbg) ss_dbg_get_isr_latency(void)
{
    ss_dbg_entry_delay = TAR;
    if (!ss_dbg_interrupted)
    {
        return 0;
    }
    else
    {
        return TAR;
    }
}
