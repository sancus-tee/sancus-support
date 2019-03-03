#include "sancus_step.h"
#include "timer.h"
#include <stdio.h>
#include <sancus/sm_support.h>
#include "sm_io.h"

void print_latency(void)
{
    int latency = get_latency();
    printf("latency: %d\n", latency);
}

int get_latency(void)
{
    TACTL = TACTL_DISABLE;
    return __ss_isr_tar_entry - sm_exit_latency;
}

int ss_dbg_entry_delay = 0;
int ss_dbg_measuring_reti_latency = 0;
int isr_reti_latency = 0;
int sm_exit_latency = 0;
int isr_interrupted_sm = 0;

/*
 * Determines isr_reti_latency (reti into interrupted module)
 * and sm_exit_latency (from interrupt to isr)
 * and starts timer, so that nemesis can be executed
 */
void sancus_step_start(void)
{
    sancus_step_init();
    __asm__("dint\n\t");
    timer_irq(INIT_LATENCY);
}

/*
 * Determines isr_reti_latency (reti into interrupted moduled)
 * and sm_exit_latency (from interrupt to isr)
 */
void sancus_step_init(void)
{
    sancus_enable(&ssdbg);
    
    ss_dbg_measuring_reti_latency = 1;
    
    timer_tsc_start();
    ss_dbg_get_info();
    
    timer_irq(ss_dbg_entry_delay);
    int isr_reti_latency_t = ss_dbg_get_info();
    /* amount of cycles in the reti logic = measured delay
                                  - record delay
                                  - duration of reti instruction
                                  + 1 (because we count amount of cycles)
    */
    isr_reti_latency = isr_reti_latency_t
                        - JMP_LENGTH
                        - RET_LENGTH
                        - RETI_LENGTH
                        + 1;
    
    timer_irq(ss_dbg_entry_delay + EXTRA_DELAY);
    ss_dbg_get_info();
    sm_exit_latency = __ss_isr_tar_entry - 3;
    printf("%d, %d, %d\n", isr_reti_latency, sm_exit_latency, ss_dbg_entry_delay);
    
    ss_dbg_measuring_reti_latency = 0;
}

void sancus_step_end(void)
{
    TACTL = TACTL_DISABLE;
}

__attribute__((naked))
void sancus_step_isr_main(void)
{
    __asm__ __volatile__(
        "cmp #0x0, &%2\n\t"
        "jz 1f\n\t"
        ";sm interrupted\n\t"
        "cmp #0x0, &%3\n\t"
        "jz 3f\n\t"
        "; measuring isr_reti_latency\n\t"
        "mov #0x4, &%0\n\t"
        "mov #0x220, &%0\n\t"
        "jmp 2f\n\t"
        "3: ; not measuring isr_reti_latency\n\t"
        "call #print_latency\n\t"
        "mov #0x4, &%0\n\t"
        "push r15\n\t"
        "mov &%4, r15\n\t"
        "add #0x8, r15 ;\n\t"
        "mov r15, &%1\n\t"
        "pop r15\n\t"
        "mov #0x212, &%0\n\t"
        "jmp 2f\n\t"
        "1: ; sm not interrupted\n\t"
        "call #timer_disable\n\t"
        "2: ret\n\t"
        :
        :
        "m"(TACTL),
        "m"(TACCR0),
        "m"(isr_interrupted_sm),
        "m"(ss_dbg_measuring_reti_latency),
        "m"(isr_reti_latency));
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
}

DECLARE_SM(ssdbg, 0x1234);

__attribute__((naked))
int SM_ENTRY(ssdbg) ss_dbg_get_info(void)
{
    /*
    ss_dbg_entry_delay = TAR;
    int rv = TAR;
    TACTL = TACTL_DISABLE;
    TACTL = TACTL_CONTINUOUS;
    return rv;
    */
    
    __asm__ __volatile__(
                "mov &%0, &ss_dbg_entry_delay\n\t"
                "mov &%0, r15\n\t"
                "mov #0x4, &%1\n\t"
                "mov #0x220, &%1\n\t"
                "sub #0x1, r15\n\t"
                "ret\n\t"
                ::"m"(TAR), "m"(TACTL):
                );
}
