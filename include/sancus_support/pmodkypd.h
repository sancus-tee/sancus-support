#ifndef SANCUS_SUPPORT_PMODKYPD_H
#define SANCUS_SUPPORT_PMODKYPD_H

#include <stdint.h>

typedef enum
{
    Key_0 =  3,
    Key_1 =  0,
    Key_2 =  4,
    Key_3 =  8,
    Key_4 =  1,
    Key_5 =  5,
    Key_6 =  9,
    Key_7 =  2,
    Key_8 =  6,
    Key_9 = 10,
    Key_A = 12,
    Key_B = 13,
    Key_C = 14,
    Key_D = 15,
    Key_E = 11,
    Key_F =  7
} PmodKypdKey;

typedef void (*key_event_cb)(PmodKypdKey);

void pmodkypd_init(key_event_cb press, key_event_cb release);
void pmodkypd_poll(void);
char pmodkypd_key_to_char(PmodKypdKey key);
int pmodkypd_key_is_pressed(PmodKypdKey key);
uint16_t pmodkypd_get_key_state(void);

#endif
