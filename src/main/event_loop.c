#include "event_loop.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "uart.h"
#include "elf.h"

typedef enum
{
    Echo = 0x00,
    Load = 0x01
} Command;

static void load_module(void)
{
    unsigned char msb = uart_read_byte();
    unsigned char lsb = uart_read_byte();
    uint16_t size = (msb << 8) | lsb;

    unsigned char* file = malloc(size);
    if (file == NULL)
    {
        while (size--)
            uart_read_byte();

        printf("Not enough memory for a %uB module\n", size);
        return;
    }

    uart_read(file, size);
    printf("Read %u bytes:\n", size);
    unsigned i;
    for (i = 0; i < size; i++)
    {
        printf("%02x ", file[i]);
        if ((i + 1) % 26 == 0)
            printf("\n");
    }

    printf("\nLoading...\n");
    ElfModule em;

    int error = elf_load(file, size, &em);
    free(file);

    if (error != 0)
    {
        puts("Error loading module");
        return;
    }

    puts("Running module");
    ((void (*)(void))em.pmem)();
}

void event_loop_start(void)
{
    while (1)
    {
        Command command = uart_read_byte();
        switch (command)
        {
            case Echo:
                uart_write_byte(uart_read_byte());
                break;

            case Load:
                load_module();
                break;

            default:
                printf("Unknown command %02x\n", command);
        }

        printf("Finished command %02x\n", command);
    }
}
