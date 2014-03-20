#include "event_loop.h"

#include <stdio.h>
#include <msp430.h>


#include "sm_control.h"
#include "uart.h"
#include "global_symtab.h"
#include "tools.h"

uint8_t key_pressed = 0;

typedef enum
{
    Echo       = 0x00,
    SmLoad     = 0x01,
    SmCall     = 0x02,
    SmIdentity = 0x03,
    LoadData   = 0x04,
    Symtab     = 0x05
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

        case SmLoad:
            sm_load();
            break;

        case SmCall:
            sm_call();
            break;

        case SmIdentity:
            sm_print_identity();
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

static void dummy_idle_callback(int idle)
{
}

void event_loop_start(idle_callback set_idle)
{
    if (set_idle == NULL)
        set_idle = dummy_idle_callback;

    set_idle(1);
    puts("Event loop started");

    while (1)
    {
        if (uart_available())
        {
            set_idle(0);
            handle_command();
            set_idle(1);
        }
    }
}
