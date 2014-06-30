#include "tools.h"

#include "uart.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint16_t read_int(void)
{
    unsigned char msb = uart_read_byte();
    unsigned char lsb = uart_read_byte();
    return (msb << 8) | lsb;
}

void write_int(uint16_t i)
{
    uart_write_byte(i >> 8);
    uart_write_byte(i & 0x00ff);
}

void write_pointer(const void* ptr)
{
    write_int((uint16_t)ptr);
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

void print_data(const unsigned char* data, size_t size)
{
    int need_nl;
    unsigned i;
    for (i = 0; i < size; i++)
    {
        need_nl = 1;
        printf("%02x ", data[i]);
        if ((i + 1) % 26 == 0)
        {
            printf("\n");
            need_nl = 0;
        }
    }

    if (need_nl)
        printf("\n");
}

static int uart_putchar(int c)
{
    uart_write_byte(c);
    return c;
}

int uart_printf(const char* str, ...)
{
    va_list ap;
    va_start(ap, str);
    vuprintf(uart_putchar, str, ap);
    va_end(ap);
}

struct ParseState
{
    uint8_t* buf;
    size_t   len;
};

ParseState* create_parse_state(uint8_t* buf, size_t len)
{
    ParseState* state = malloc(sizeof(ParseState));
    state->buf = buf;
    state->len = len;
    return state;
}

void free_parse_state(ParseState* state)
{
    free(state);
}

static void advance_state(ParseState* state, size_t len)
{
    state->buf += len;
    state->len -= len;
}

int parse_byte(ParseState* state, uint8_t* byte)
{
    if (state->len < 1)
        return 0;

    *byte = state->buf[0];
    advance_state(state, 1);
    return 1;
}

int parse_int(ParseState* state, uint16_t* i)
{
    if (state->len < 2)
        return 0;

    uint8_t msb = state->buf[0];
    uint8_t lsb = state->buf[1];
    *i = (msb << 8) | lsb;
    advance_state(state, 2);
    return 1;
}

int parse_string(ParseState* state, char** str)
{
    uint8_t* end = memchr(state->buf, 0x00, state->len);

    if (end == NULL)
        return 0;

    size_t str_len = end - state->buf;
    *str = state->buf;
    advance_state(state, str_len + 1);
    return 1;
}

int parse_raw_data(ParseState* state, size_t len, uint8_t** buf)
{
    if (state->len < len)
        return 0;

    *buf = state->buf;
    advance_state(state, len);
    return 1;
}

int parse_all_raw_data(ParseState* state, uint8_t** buf, size_t* len)
{
    *buf = state->buf;
    *len = state->len;
    advance_state(state, state->len);
    return 1;
}

#define PRINTF_BUF_LEN 64

static printf_cb callback = NULL;
static uint8_t printf_buf[PRINTF_BUF_LEN] = {};
static size_t printf_buf_idx = 0;

int cb_printf_init(printf_cb cb)
{
    cb_printf_finish();
    callback = cb;

    if (printf_buf == NULL)
        return 0;

    printf_buf_idx = 0;
    return 1;
}

void cb_printf_finish(void)
{
    if (printf_buf_idx > 0)
        callback(printf_buf, printf_buf_idx);

    callback = NULL;
    printf_buf_idx = 0;
}

static int buf_putchar(int c)
{
    if (printf_buf == NULL)
        return EOF;

    printf_buf[printf_buf_idx++] = c;

    if (printf_buf_idx == PRINTF_BUF_LEN)
    {
        callback(printf_buf, PRINTF_BUF_LEN);
        printf_buf_idx = 0;
    }

    return c;
}

int cb_printf(const char* str, ...)
{
    va_list ap;
    va_start(ap, str);
    vuprintf(buf_putchar, str, ap);
    va_end(ap);
}
