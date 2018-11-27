#include "sm_io.h"
#include "uart.h"
#include <sancus/sm_support.h>
#include <msp430.h>

void msp430_io_init(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    uart_init();
    puts("\n------\n\n");
}

int putchar(int c)
{
    if (c == '\n')
        putchar('\r');

    uart_write_byte(c);
    return c;
}

void pr_sm_info(struct SancusModule *sm)
{
    ASSERT(sm->id && (sancus_get_id(sm->public_start) == sm->id));
    
    printf("SM %s with ID %d enabled\t: 0x%.4x 0x%.4x 0x%.4x 0x%.4x\n",
        sm->name, sm->id,
        (uintptr_t) sm->public_start, (uintptr_t) sm->public_end,
        (uintptr_t) sm->secret_start, (uintptr_t) sm->secret_end);
}

void __attribute__((noinline)) printf0(const char* str)
{
    printf("%s", str);
}

void __attribute__((noinline)) printf1(const char* fmt, int arg1)
{
    printf(fmt, arg1);
}

void __attribute__((noinline)) printf2(const char* fmt, int arg1, int arg2)
{
    printf(fmt, arg1, arg2);
}

void __attribute__((noinline)) printf3(const char* fmt, int arg1, int arg2, int arg3)
{
    printf(fmt, arg1, arg2, arg3);
}

void stop_violation(void)
{
    puts("\t--> SM VIOLATION DETECTED; exiting...\n");
    EXIT();

    pr_info("should never reach here..");
    while(1);
}

__attribute__ ((naked))
__attribute__((interrupt(SM_VECTOR)))
__attribute__((optimize("-O3")))
/* ^^ NOTE: Agressively enable optimizations to prevent GCC from allocating
 * a stack frame: https://sourceforge.net/p/mspgcc/support-requests/27/ */
void violation_isr(void)
{
    asm(    "mov &__unprotected_sp, r1                  \n\t"   \
            "call #stop_violation                       \n\t"   \
       );
}
