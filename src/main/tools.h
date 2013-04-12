#ifndef MSP_SUPPORT_TOOLS_H
#define MSP_SUPPORT_TOOLS_H

#include <stdlib.h>

uint16_t read_int(void);
char* read_string(void);
void print_data(const unsigned char* data, size_t size);

#endif

