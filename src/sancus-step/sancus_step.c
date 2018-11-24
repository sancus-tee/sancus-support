#include "sancus_step.h"
#include "timer.h"

void*    __ss_isr_sp = &__ss_isr_stack[ISR_STACK_SIZE-1];

void print_latency(void)
{
    TACTL = TACTL_DISABLE;
    int latency = __ss_isr_tar_entry - HW_IRQ_LATENCY - 1;
    printf("latency: %d\n", latency);
}
