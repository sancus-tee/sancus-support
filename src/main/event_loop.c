#include "event_loop.h"

#include <stdio.h>
#include <msp430.h>


#include "spm_control.h"
#include "uart.h"
#include "global_symtab.h"
#include "tools.h"

uint8_t key_pressed = 0;

typedef enum
{
    Echo        = 0x00,
    SpmLoad     = 0x01,
    SpmCall     = 0x02,
    SpmIdentity = 0x03,
    LoadData    = 0x04,
    Symtab      = 0x05
} Command;

static void load_data(void)
{
    void* address = (void*)read_int();
    size_t size = read_int();
    uart_read(address, size);
    printf("Loaded data at address %p:\n", address);
    print_data(address, size);
}

static void handle_command(void)
{
    Command command = uart_read_byte();
    switch (command)
    {
        case Echo:
            uart_write_byte(uart_read_byte());
            break;

        case SpmLoad:
            spm_load();
            break;

        case SpmCall:
            spm_call();
            break;

        case SpmIdentity:
            spm_print_identity();
            break;

        case LoadData:
            load_data();
            break;

        case Symtab:
            print_global_symbols(uart_printf);
            break;

        default:
            printf("Unknown command %02x\n", command);
    }

    printf("Finished command %02x\n", command);
}

void event_loop_start(void)
{
    puts("Event loop started");
    while (1)
    {
        if (uart_available())
            handle_command();
    }
}

