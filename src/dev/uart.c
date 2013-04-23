#include <msp430.h>

#include "uart.h"
#include "uart_hardware.h"

static volatile unsigned char rxdata;
static volatile int data_ready = 0;

void uart_init(void )
{
    UART_BAUD = BAUD;
    UART_CTL = UART_EN | UART_IEN_RX;
    UART2_BAUD = BAUD;
    UART2_CTL = UART_EN;
}

int uart_available(void)
{
    return data_ready;
}

void uart_write_byte(unsigned char b)
{
    // wait while TX buffer is full
    while (UART_STAT & UART_TX_FULL) {}

    // write byte to TX buffer
    UART_TXD = b;
}

void uart2_write_byte(unsigned char b)
{
    // wait while TX buffer is full
    while (UART2_STAT & UART_TX_FULL) {}

    // write byte to TX buffer
    UART2_TXD = b;
}

void uart_write(const unsigned char* buf, size_t size)
{
    while (size--)
        uart_write_byte(*buf++);
}

unsigned char uart_read_byte(void)
{
    while (!data_ready) {}
    data_ready = 0;
    return rxdata;
}

void uart_read(unsigned char* buf, size_t size)
{
    while (size--)
        *buf++ = uart_read_byte();
}

static void __attribute__((interrupt(UART_RX_VECTOR))) uart_receive(void)
{
    // Read the received data
    rxdata = UART_RXD;
    data_ready = 1;

    // Clear the receive pending flag
    UART_STAT = UART_RX_PND;
}

