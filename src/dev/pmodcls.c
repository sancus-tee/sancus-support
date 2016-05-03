#include "pmodcls.h"

#include "uart_hardware.h"

#include <stdint.h>

#define LCD_UART_BAUDRATE 2083 // 9600 baud

#define LCD_UART_CTL  (*(volatile uint8_t*)  0x0090)
#define LCD_UART_STAT (*(volatile uint8_t*)  0x0091)
#define LCD_UART_BAUD (*(volatile uint16_t*) 0x0092)
#define LCD_UART_TXD  (*(volatile uint8_t*)  0x0094)
#define LCD_UART_RXD  (*(volatile uint8_t*)  0x0095)

static void lcd_uart_write_byte(uint8_t byte)
{
    // wait while TX buffer is full
    while (LCD_UART_STAT & UART_TX_FULL) {}

    // write byte to TX buffer
    LCD_UART_TXD = byte;

    // this allows the PmodCLS to keep up. the value is determined through
    // experimentation because Digilent was nice enough to document this
    // behavior...
    __delay_cycles(20000);
}

int pmodcls_putchar(int c)
{
    lcd_uart_write_byte(c);
    return c;
}

static void lcd_uart_write(const char* str)
{
    while (*str != '\0')
        lcd_uart_write_byte(*str++);
}

static void pmodcls_write_command(const char* cmd_str)
{
    static const char* cmd_prefix = "\x1b[";
    lcd_uart_write(cmd_prefix);
    lcd_uart_write(cmd_str);
}

void pmodcls_init(void)
{
    LCD_UART_BAUD = LCD_UART_BAUDRATE;
    LCD_UART_CTL = UART_EN;
    pmodcls_clear();
}

void pmodcls_write(const char* text)
{
    lcd_uart_write(text);
}

void pmodcls_clear(void)
{
    pmodcls_write_command("j");
}

void pmodcls_set_wrap_mode(PmodClsWrapMode mode)
{
    char cmd[] = {mode == PmodClsWrapAt16 ? '0' : '1', 'h', '\0'};
    pmodcls_write_command(cmd);
}
