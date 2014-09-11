#include "ps2.h"
#include "ps2_hardware.h"

#include "private/debug.h"

#include <stdio.h>

static ps2_calback_t cb_press;
static ps2_calback_t cb_release;

int ps2_init(ps2_calback_t press, ps2_calback_t release)
{
    cb_press = press;
    cb_release = release;
    return 1;
}

static void __attribute__((interrupt(PS2_RX_VECTOR))) ps2_receive(void)
{
    uint8_t stat = PS2_STAT;
    uint8_t data = PS2_DATA;
    int released = stat & PS2_RELEASED;

    if (released)
        cb_release(data);
    else
        cb_press(data);

    DBG_VAR(const char* type = released ? "Released" : "Pressed");
    DBG_PRINTF("PS2: %s %02x (%04x)\n", type, data, stat);
}

