#include "tools.h"

#include "uart.h"

#include <stdlib.h>
#include <string.h>

uint16_t read_int(void)
{
    unsigned char msb = uart_read_byte();
    unsigned char lsb = uart_read_byte();
    return (msb << 8) | lsb;
}

char* read_string(void)
{
    size_t a_size = 0;
    size_t s_size = 0;
    char* str = NULL;

    while (1)
    {
        if (a_size == s_size)
        {
            a_size += 8;
            char* new_str = malloc(a_size);

            if (new_str == NULL)
            {
                free(str);
                return NULL;
            }

            strcpy(new_str, str);
            free(str);
            str = new_str;
        }

        char c = uart_read_byte();
        str[s_size++] = c;

        if (c == '\0')
            return str;
    }
}

