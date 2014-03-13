#ifndef SANCUS_SUPPORT_PS2_H
#define SANCUS_SUPPORT_PS2_H

#include <stdint.h>

typedef uint8_t scancode_t;
typedef void (*ps2_calback_t)(scancode_t);

int ps2_init(ps2_calback_t press, ps2_calback_t release);

#endif

