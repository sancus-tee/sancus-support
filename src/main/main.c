#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
#include "uart.h"
#include "ps2.h"

#include "event_loop.h"

extern char _etext;
char* rom = &_etext;

volatile int received = 0;
volatile unsigned char uart_data;

void init_io()
{
    // disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // all interrupts on positive edge
    P1IES = 0;
    P2IES = 0;

    // disable interrupts on all ports
    P1IE = 0;
    P2IE = 0;

    // general purpose IO on all ports
    P1SEL = 0;
    P2SEL = 0;
    P3SEL = 0;
}

void keypress(scancode_t sc)
{
    uint8_t data[] = {0x00, sc};
    uart_write(data, sizeof(data));
}

void keyrelease(scancode_t sc)
{
    uint8_t data[] = {0x01, sc};
    uart_write(data, sizeof(data));
}

int main()
{
    init_io();
    timer_init();
    uart_init();
    ps2_init(keypress, keyrelease);
    __eint();

    puts("main() started");
    event_loop_start(NULL);
    puts("main() done");
    while (1) {}
}

#if __GNUC__ >= 5
int __swbuf_r(struct _reent * r, int c, FILE * f)
#else
int putchar(int c)
#endif
{
    if (c == '\n')
        uart2_write_byte('\r');

    uart2_write_byte(c);
    return c;
}

