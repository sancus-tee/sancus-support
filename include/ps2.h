#ifndef MSP_SUPPORT_PS2_H
#define MSP_SUPPORT_PS2_H

#include <stdint.h>

typedef uint8_t scancode_t;
typedef void (*ps2_calback_t)(scancode_t);

int ps2_init(ps2_calback_t cb);

#endif

