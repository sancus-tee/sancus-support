#include "timer.h"

#include <msp430.h>

void timer_init(void)
{
    TACTL = TASSEL_2 | // select SMCLK
            ID_0     | // /1 clock divider
            MC_0     | // Stop mode (timer halted)
            TACLR;     // clear timer

    // disable capture mode
    TACCTL0 = CM_0;
    TACCTL1 = CM_0;
    TACCTL2 = CM_0;

    // start timer (up mode)
    TACTL |= MC_2;
}

