#ifndef MSP_SUPPORT_UART_H
#define MSP_SUPPORT_UART_H

#include <stddef.h>

void uart_init(void);
int uart_available(void);
void uart_write_byte(unsigned char b);
void uart2_write_byte(unsigned char b);
void uart_write(const unsigned char* buf, size_t size);
unsigned char uart_read_byte(void);
void uart_read(unsigned char* buf, size_t size);

#endif
