#include "event_loop.h"

#include <stdio.h>

#include "spm_control.h"
#include "uart.h"
#include "global_symtab.h"

typedef enum
{
    Echo =   0x00,
    Load =   0x01,
    Call =   0x02,
    Symtab = 0x03
} Command;

void event_loop_start(void)
{
    while (1)
    {
        puts("Waiting for command...");
        Command command = uart_read_byte();
        switch (command)
        {
            case Echo:
                uart_write_byte(uart_read_byte());
                break;

            case Load:
                spm_load();
                break;

            case Call:
                spm_call();
                break;

            case Symtab:
                print_global_symbols();
                break;

            default:
                printf("Unknown command %02x\n", command);
        }

        printf("Finished command %02x\n", command);
    }
}

