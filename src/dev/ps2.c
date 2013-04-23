#include "ps2.h"
#include "ps2_hardware.h"

#include <stdio.h>

ps2_calback_t callback;

int ps2_init(ps2_calback_t cb)
{
    callback = cb;
}

static void __attribute__((interrupt(PS2_RX_VECTOR))) ps2_receive(void)
{
    uint8_t stat = PS2_STAT;
    uint8_t data = PS2_DATA;
    int released = stat & PS2_RELEASED;

    if (!released)
        callback(data);

    const char* type = released ? "Released" : "Pressed";
    printf("PS2: %s %02x\n", type, data);
}

