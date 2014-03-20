#include "sm_control.h"

#include "uart.h"
#include "elf.h"
#include "tools.h"
#include "global_symtab.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sancus/sm_support.h>

/*#define NO_PROTECT*/

#ifdef NO_PROTECT
static sm_id next_id = 1;
#endif

typedef struct SmList
{
    struct SancusModule sm;
    ElfModule*          em;
    struct SmList*      next;
} SmList;

SmList* sm_head = NULL;

static struct SancusModule* find_sm(sm_id id)
{
    SmList* current = sm_head;
    while (current != NULL && current->sm.id != id)
        current = current->next;

    if (current == NULL)
    {
        printf("No SPM with ID 0x%x\n", id);
        return NULL;
    }

    return &current->sm;
}

static void* get_sm_symbol(const char* sm_name, char* which)
{
    char* name = malloc(strlen("__sm_") + strlen(sm_name) +
                        strlen(which) + 2);
    if (name == NULL)
        return NULL;

    strcpy(name, "__sm_");
    strcat(name, sm_name);
    strcat(name, "_");
    strcat(name, which);
    void* sym = get_global_symbol_value(name);
    free(name);
    return sym;
}

static void append_sm_list(SmList* sm)
{
    SmList** current = &sm_head;

    while (*current != NULL)
        current = &(*current)->next;

    *current = sm;
    sm->next = NULL;
}

static int register_sm(char* name, uint16_t vendor_id, ElfModule* em)
{
    SmList* sm_list = malloc(sizeof(SmList));
    sm_list->em = em;

    struct SancusModule* sm = &sm_list->sm;
    sm->id = 0;
    sm->vendor_id = vendor_id;
    sm->name = name;
    sm->public_start = get_sm_symbol(name, "public_start");
    sm->public_end = get_sm_symbol(name, "public_end");
    sm->secret_start = get_sm_symbol(name, "secret_start");
    sm->secret_end = get_sm_symbol(name, "secret_end");

    if (sm->public_start == NULL || sm->public_end == NULL ||
        sm->secret_start == NULL || sm->secret_end == NULL)
    {
        puts("Failed to find SPM symbols");
        free(sm_list);
        return 0;
    }

#ifdef NO_PROTECT
    sm->id = next_id++;
#else
    if (!sancus_enable(sm))
    {
        puts("Protecting SPM failed");
        free(sm_list);
        return 0;
    }
#endif

    append_sm_list(sm_list);

    printf("Registered SPM %s with id 0x%x for vendor 0x%x\n",
           name, sm->id, vendor_id);
    printf(" - Public: [%p, %p]\n", sm->public_start, sm->public_end);
    printf(" - Secret: [%p, %p]\n", sm->secret_start, sm->secret_end);
    return 1;
}

int sm_register_existing(struct SancusModule* sm)
{
    SmList* sm_list = malloc(sizeof(SmList));
    sm_list->sm = *sm;
    sm_list->em = NULL;
    append_sm_list(sm_list);
    return 1;
}

void sm_load(void)
{
    int error = 0;
    char* name = read_string();
    if (name == NULL)
        error = 1;

    uint16_t vendor_id = read_int();
    uint16_t size = read_int();
    unsigned char* file = malloc(size);
    if (file == NULL || error)
    {
        while (size--)
            uart_read_byte();

        printf("Not enough memory for a %uB module\n", size);
        free(name);
        return;
    }

    uart_read(file, size);
    printf("Accepted SPM %s for vendor %u\n", name, vendor_id);
    printf("Read %u bytes:\n", size);
    print_data(file, size);
    printf("Loading...\n");

    ElfModule* em = elf_load(file);
    free(file);

    if (em == NULL)
    {
        puts("Error loading module");
        return;
    }

    puts("Module successfully loaded");

    if (!register_sm(name, vendor_id, em))
    {
        puts("Registering SPM failed");
        return;
    }
}

typedef struct
{
    void*    entry;
    uint16_t index;
    uint16_t nargs;
    uint16_t arg1;
    uint16_t arg2;
    uint16_t arg3;
    uint16_t arg4;
} CallInfo;

// we need a bit of optimization here to make GCC not use all available
// registers
static void __attribute__((optimize("-O1"))) enter_sm(CallInfo* ci)
{
    uint16_t args_left = ci->nargs;

    asm("tst  %0\n\t"
        "jz   1f\n\t"
        "mov  %1, r15\n\t"
        "dec  %0\n\t"
        "jz   1f\n\t"
        "mov  %2, r14\n\t"
        "dec  %0\n\t"
        "jz   1f\n\t"
        "mov  %3, r13\n\t"
        "dec  %0\n\t"
        "jz   1f\n\t"
        "mov  %4, r12\n\t"
        "dec  %0\n\t"

        "1:\n\t"
        "mov  r1, &__unprotected_sp\n\t"
        "mov  %5, r6\n\t"
        "mov  #1f, r7\n\t"
        "br   %6\n\t"
        "1:\n\t"
        "mov  &__unprotected_sp, r1\n\t"
        :
        : "r"(args_left), "m"(ci->arg1), "m"(ci->arg2), "m"(ci->arg3),
          "m"(ci->arg4), "m"(ci->index), "m"(ci->entry)
        : "r6", "r7", "r12", "r13", "r14", "r15");
}

void sm_call(void)
{
    uint16_t id = read_int();
    uint16_t index = read_int();
    uint16_t nargs = read_int();

    CallInfo ci;
    void* arg_buf = NULL;

    switch (nargs)
    {
        case 0:
            ci.nargs = 0;
            break;

        case 1:
            ci.nargs = 1;
            ci.arg1 = read_int();
            break;

        case 2:
            ci.nargs = 2;
            ci.arg1 = read_int();
            ci.arg2 = read_int();
            break;

        case 3:
            ci.nargs = 3;
            ci.arg1 = read_int();
            ci.arg2 = read_int();
            ci.arg3 = read_int();
            break;

        case 4:
            ci.nargs = 4;
            ci.arg1 = read_int();
            ci.arg2 = read_int();
            ci.arg3 = read_int();
            ci.arg4 = read_int();
            break;

        default:
            arg_buf = malloc(nargs);
            if (arg_buf == NULL)
            {
                puts("Out of memory");
                while (nargs--)
                    uart_read_byte();
                return;
            }

            uart_read(arg_buf, nargs);

            ci.nargs = 2;
            ci.arg1 = (uint16_t)arg_buf;
            ci.arg2 = nargs;
            break;
    }

    struct SancusModule* sm = find_sm(id);
    if (sm == NULL)
        return;

    ci.entry = sm->public_start;
    ci.index = index;

    printf("Accepted call to SPM %s, index %u, args %u\n",
           sm->name, index, nargs);

    enter_sm(&ci);
    free(arg_buf);
}

void sm_print_identity(void)
{
    sm_id id = read_int();
    struct SancusModule* sm = find_sm(id);
    if (sm == NULL)
        return;

    printf("Identity of SPM %s:\n", sm->name);
    print_data(sm->public_start,
               (char*)sm->public_end - (char*)sm->public_start);
    print_data((char*)&sm->public_start, 2);
    print_data((char*)&sm->public_end, 2);
    print_data((char*)&sm->secret_start, 2);
    print_data((char*)&sm->secret_end, 2);
}

