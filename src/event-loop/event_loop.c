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

static void load_sancus_module(ParseState* state)
{
    static const char* error_prefix = "Error reading SmLoad packet";

    sm_id ret_id = 0;
    uint8_t buf[2];
    char* name = NULL;

    if (!parse_string(state, &name))
    {
        printf("%s: Expected SM name\n", error_prefix);
        goto out;
    }

    uint16_t vid;

    if (!parse_int(state, &vid))
    {
        printf("%s: Expected vendor ID\n", error_prefix);
        goto out;
    }

    uint16_t size;

    if (!parse_int(state, &size))
    {
        printf("%s: Expected SM size\n", error_prefix);
        goto out;
    }

    uint8_t* file;

    if (!parse_raw_data(state, size, &file))
    {
        printf("%s: Expected %u bytes of data\n", error_prefix, size);
        goto out;
    }

    printf("Loading SM %s for vendor %u with size %u\n", name, vid, size);
    ret_id = sm_load(file, name, vid);

out:
    buf[0] = ret_id >> 8;
    buf[1] = ret_id & 0xff;
    packet_write(buf, sizeof(buf));
}

static void call_sancus_module(ParseState* state)
{
    static const char* error_prefix = "Error reading SmCall packet";

    uint16_t id, index, nargs;

    if (!parse_int(state, &id)    ||
        !parse_int(state, &index) ||
        !parse_int(state, &nargs))
    {
        printf("%s: Incorrect header\n", error_prefix);
        return;
    }

    uint16_t args[4];
    uint8_t* arg_buf = NULL;

    switch (nargs)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            for (uint16_t i = 0; i < nargs; i++)
            {
                if (!parse_int(state, &args[i]))
                {
                    printf("%s: Failed to read argument %u\n", error_prefix, i);
                    return;
                }
            }

            break;

        default:
            if (!parse_raw_data(state, nargs, &arg_buf))
            {
                printf("%s: Failed to read raw arguments\n", error_prefix);
                return;
            }

            nargs = 2;
            args[0] = (uint16_t)arg_buf;
            args[1] = nargs;
            break;
    }

    sm_call(id, index, args, nargs);
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
            load_sancus_module(state);
            break;

        case SmCall:
            call_sancus_module(state);
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
