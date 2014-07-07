#ifndef SANCUS_SUPPORT_TOOLS_H
#define SANCUS_SUPPORT_TOOLS_H

#include <stdlib.h>

uint16_t read_int(void);
void write_int(uint16_t i);
void write_pointer(const void* ptr);
char* read_string(void);
void print_data(const unsigned char* data, size_t size);
int __attribute__((format(printf, 1, 2))) uart_printf(const char* str, ...);

typedef struct ParseState ParseState;

ParseState* create_parse_state(uint8_t* buf, size_t len);
void free_parse_state(ParseState* state);
int parse_byte(ParseState* state, uint8_t* byte);
int parse_int(ParseState* state, uint16_t* i);
int parse_string(ParseState* state, char** str);
int parse_raw_data(ParseState* state, size_t len, uint8_t** buf);
int parse_all_raw_data(ParseState* state, uint8_t** buf, size_t* len);

typedef int (*printf_cb)(uint8_t*, size_t);

int cb_printf_init(printf_cb cb);
void cb_printf_finish(void);
int __attribute__((format(printf, 1, 2))) cb_printf(const char* str, ...);

#endif
