#include "sancus_step.h"
#include "timer.h"
#include <stdio.h>

void*    __ss_isr_sp = &__ss_isr_stack[ISR_STACK_SIZE-1];

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

/* ============TIMING MODULE===========*/
void SM_ENTRY(spy) spy_enter(void)
{
    // Setup interrupt flag
    __asm__("");
    // Interrupt routine handles interrupt and resets timer
    // Save timer value to spy_IRQ_delay upon entry
    __asm__("mov &%0, &%1":"m"(spy_IRQ_delay):"m"(TAR):);
}
