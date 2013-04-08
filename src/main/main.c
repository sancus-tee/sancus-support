#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
#include "uart.h"

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

int main()
{
    init_io();
    timer_init();
    uart_init();
    __eint();

    puts("main() started");
    /*rom[10] = 'A';*/
    /*putchar(rom[10]);*/
    /*putchar('\n');*/
    strcpy(&_etext, "test");
    printf("End of ROM: %x, %s\n", &_etext, &_etext);
    event_loop_start();
    puts("main() done");
    while (1) {}
}

int putchar(int c)
{
    if (c == '\n')
        uart_write_byte('\r');

    uart_write_byte(c);
    return c;
}

