#include "event_loop.h"

#include <stdio.h>
#include <string.h>
#include <msp430.h>

#include "sm_control.h"
#include "packet.h"
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
    SmLdInfo   = 0x05,
    PrintData  = 0x06,
} Command;

static void echo(ParseState* state)
{
    uint8_t* buf;
    size_t len;

    if (parse_all_raw_data(state, &buf, &len))
        packet_write(buf, len);
    else
        puts("Error reading Echo packet");
}

static void load_data(ParseState* state)
{
    static const char* error_prefix = "Error reading LoadData packet";

    void* address;
    size_t size;

    if (!parse_int(state, (uint16_t*)&address) || !parse_int(state, &size))
    {
        printf("%s: Wrong header\n", error_prefix);
        return;
    }

    uint8_t* buf;

    if (!parse_raw_data(state, size, &buf))
    {
        printf("%s: Not enough data\n", error_prefix);
        return;
    }

    memcpy(address, buf, size);

    printf("Loaded data at address %p:\n", address);
    print_data(address, size);
}

static void print_sm_ld_info(ParseState* state)
{
    static const char* error_prefix = "Error handling SmLdInfo packet";

    sm_id id;

    if (!parse_int(state, &id))
    {
        printf("%s: Failed to read ID\n", error_prefix);
        return;
    }

    if (!cb_printf_init(packet_write))
    {
        printf("%s: Out of memory 1\n", error_prefix);
        return;
    }

    if (id == 0)
        print_global_symbols(cb_printf);
    else
    {
        ElfModule* module = sm_get_elf_by_id(id);

        if (module == NULL)
            printf("%s: No module with ID 0x%x\n", error_prefix, id);
        else
        {
            print_global_symbols(cb_printf);
            print_module_sections(module, cb_printf);
        }
    }

    cb_printf_finish();
    return;
}

static void print_uart_data(ParseState* state)
{
    static const char* error_prefix = "Error handling PrintData packet";

    size_t len;

    if (!parse_int(state, &len))
    {
        printf("%s: Failed to read data length\n", error_prefix);
        return;
    }

    uint8_t* buf;

    if (!parse_raw_data(state, len, &buf))
    {
        printf("%s: Failed to read %u bytes of data\n", error_prefix, len);
        return;
    }

    for (size_t i = 0; i < len; i++)
        printf("%02x ", buf[i]);

    printf("\n");
}

static void handle_command(void)
{
    Packet* packet = packet_get_next();
    ParseState* state = create_parse_state(packet->data, packet->len);
    uint8_t command;

    if (!parse_byte(state, &command))
        goto free_mem;

    if (!packet_start())
        goto free_mem;

    printf("handle_command %02x\n", command);

    switch (command)
    {
        case Echo:
            echo(state);
            break;

        case SmLoad:
            sm_load(state);
            break;

        case SmCall:
            sm_call(state);
            break;

        case SmIdentity:
            sm_print_identity(state);
            break;

        case LoadData:
            load_data(state);
            break;

        case SmLdInfo:
            print_sm_ld_info(state);
            break;

        case PrintData:
            print_uart_data(state);
            break;

        default:
            printf("Unknown command %02x\n", command);
    }

    if (!packet_finish())
        goto free_mem;

    printf("Finished command %02x\n", command);

free_mem:
    packet_free(packet);
    free_parse_state(state);
}

static void dummy_tick_callback()
{
}

void event_loop_start(tick_callback tick)
{
    if (tick == NULL)
        tick = dummy_tick_callback;

    puts("Event loop started");

    while (1)
    {
        if (packet_available())
            handle_command();

        tick();
    }
}
