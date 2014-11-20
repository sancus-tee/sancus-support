#include "pmodkypd.h"

#include <stdlib.h>
#include <msp430.h>

typedef uint16_t key_state_t;

static const int NUM_KEYS = 16;

static key_state_t current_key_state = 0;
static key_event_cb press_cb = NULL;
static key_event_cb release_cb = NULL;

static key_state_t read_key_state(void)
{
    key_state_t state = 0;

    for (int i = 0; i < 4; i++)
    {
        // pull the column pin low
        P2OUT = ~(1 << i);

        // if we don't wait a couple of cycles, we sometimes seem to read old
        // values from the row pins
        __delay_cycles(5);

        // read the state of the current column (the row pins) and shift it
        // into the state
        uint8_t col = P2IN >> 4;
        state |= (col << (4 * i));
    }

    // since the row pins are active low, we invert the state to get a more
    // natural representation
    return ~state;
}

static int key_is_pressed(key_state_t key_state, PmodKypdKey key)
{
    return key_state & (1 << key);
}

static void on_press(PmodKypdKey key)
{
    if (press_cb != NULL)
        press_cb(key);
}

static void on_release(PmodKypdKey key)
{
    if (release_cb != NULL)
        release_cb(key);
}

void pmodkypd_init(key_event_cb press, key_event_cb release)
{
    press_cb = press;
    release_cb = release;

    P2SEL = 0x00;

    // the column pins are mapped to P2OUT[0:3] and the row pins to P2IN[4:7]
    P2DIR = 0x0f;
}

void pmodkypd_poll(void)
{
    key_state_t new_key_state = read_key_state();

    for (int key = 0; key < NUM_KEYS; key++)
    {
        int was_pressed = key_is_pressed(current_key_state, key);
        int is_pressed = key_is_pressed(new_key_state, key);

        if (!was_pressed && is_pressed)
            on_press(key);
        else if (was_pressed && !is_pressed)
            on_release(key);
    }

    current_key_state = new_key_state;
}

char pmodkypd_key_to_char(PmodKypdKey key)
{
    static char keymap[] = {
        '1', '4', '7', '0',
        '2', '5', '8', 'F',
        '3', '6', '9', 'E',
        'A', 'B', 'C', 'D'
    };

    return keymap[key];
}

int pmodkypd_key_is_pressed(PmodKypdKey key)
{
    return key_is_pressed(current_key_state, key);
}

uint16_t pmodkypd_get_key_state(void)
{
    return current_key_state;
}
