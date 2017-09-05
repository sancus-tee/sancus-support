#include "led_digits.h"
#include <stdint.h>

// MMIO registers for led digit segments:
// (MSB-1) g f e d c b a (LSB)
#define LED_BASE    0x0090
#define LED1        *(volatile uint8_t*)(LED_BASE+0)
#define LED2        *(volatile uint8_t*)(LED_BASE+1)
#define LED3        *(volatile uint8_t*)(LED_BASE+2)
#define LED4        *(volatile uint8_t*)(LED_BASE+3)
#define LED5        *(volatile uint8_t*)(LED_BASE+4)
#define LED6        *(volatile uint8_t*)(LED_BASE+5)
#define LED7        *(volatile uint8_t*)(LED_BASE+6)
#define LED8        *(volatile uint8_t*)(LED_BASE+7)
#define _BAD        0x00

const uint8_t led_digit_map[] = {
    /*0=*/ 0x3f, 0x06, 0x5b, 0x4f, 0x66,
    /*5=*/ 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

const uint8_t led_alpha_map[] = {
    /*A=*/ 0x77, 0x7c, 0x39, 0x5e, 0x79,
    /*F=*/ 0x71, 0x3d, 0x74, 0x30, 0x1e,
    /*K=*/ _BAD, 0x38, _BAD, 0x54, 0x5c,
    /*P=*/ 0x73, _BAD, 0x50, 0x6d, 0x78,
    /*U=*/ 0x1c, _BAD, _BAD, _BAD, 0x6e,
    /*Z=*/ _BAD
};

uint8_t led_from_char(char c)
{
    if (c >= '0' && c <= '9')
        return led_digit_map[c-'0'];

    if (c >= 'A' && c <= 'Z')
        c = 'a' + (c-'A'); 

    if (c >= 'a' && c <= 'z')
        return led_alpha_map[c-'a'];

    return _BAD;
}

void led_digits_update(char c1, char c2, char c3, char c4,
                       char c5, char c6, char c7, char c8)
{
    LED1 = led_from_char(c1);
    LED2 = led_from_char(c2);
    LED3 = led_from_char(c3);
    LED4 = led_from_char(c4);
    LED5 = led_from_char(c5);
    LED6 = led_from_char(c6);
    LED7 = led_from_char(c7);
    LED8 = led_from_char(c8);
}
