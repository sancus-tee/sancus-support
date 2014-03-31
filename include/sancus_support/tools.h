#ifndef SANCUS_SUPPORT_TOOLS_H
#define SANCUS_SUPPORT_TOOLS_H

#include <stdlib.h>

uint16_t read_int(void);
void write_int(uint16_t i);
void write_pointer(const void* ptr);
char* read_string(void);
void print_data(const unsigned char* data, size_t size);
int __attribute__((format(printf, 1, 2))) uart_printf(const char* str, ...);

#endif

