#include "sm_control.h"

#include "uart.h"
#include "link.h"
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

static SmList* find_sm_list(sm_id id)
{
    SmList* current = sm_head;
    while (current != NULL && current->sm.id != id)
        current = current->next;

    if (current == NULL)
    {
        printf("No SM with ID 0x%x\n", id);
        return NULL;
    }

    return current;
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
    void* sym;
    int ok = get_global_symbol_value(name, &sym);
    free(name);
    return ok ? sym : NULL;
}

static void append_sm_list(SmList* sm)
{
    SmList** current = &sm_head;

    while (*current != NULL)
        current = &(*current)->next;

    *current = sm;
    sm->next = NULL;
}

static struct SancusModule* register_sm(char* name, uint16_t vendor_id,
                                        ElfModule* em)
{
    SmList* sm_list = malloc(sizeof(SmList));
    sm_list->em = em;

    struct SancusModule* sm = &sm_list->sm;
    sm->id = 0;
    sm->vendor_id = vendor_id;
    sm->name = strdup(name);
    sm->public_start = get_sm_symbol(name, "public_start");
    sm->public_end = get_sm_symbol(name, "public_end");
    sm->secret_start = get_sm_symbol(name, "secret_start");
    sm->secret_end = get_sm_symbol(name, "secret_end");

    if (sm->public_start == NULL || sm->public_end == NULL ||
        sm->secret_start == NULL || sm->secret_end == NULL)
    {
        puts("Failed to find SPM symbols");
        free(sm_list);
        return NULL;
    }

#ifdef NO_PROTECT
    sm->id = next_id++;
#else
    if (!sancus_enable(sm))
    {
        puts("Protecting SPM failed");
        free(sm_list);
        return NULL;
    }
#endif

    append_sm_list(sm_list);

    printf("Registered SPM %s with id 0x%x for vendor 0x%x\n",
           name, sm->id, vendor_id);
    printf(" - Public: [%p, %p]\n", sm->public_start, sm->public_end);
    printf(" - Secret: [%p, %p]\n", sm->secret_start, sm->secret_end);
    return sm;
}

int sm_register_existing(struct SancusModule* sm)
{
    SmList* sm_list = malloc(sizeof(SmList));
    sm_list->sm = *sm;
    sm_list->em = NULL;
    append_sm_list(sm_list);
    return 1;
}

void sm_load(ParseState* state)
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

    uint16_t vendor_id;

    if (!parse_int(state, &vendor_id))
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

    printf("Accepted SPM %s for vendor %u\n", name, vendor_id);
    printf("Read %u bytes:\n", size);
    print_data(file, size);
    printf("Loading...\n");

    ElfModule* em = elf_load(file);

    if (em == NULL)
    {
        puts("Error loading module");
        goto out;
    }

    puts("Module successfully loaded");

    struct SancusModule* sm = register_sm(name, vendor_id, em);

    if (sm == NULL)
    {
        puts("Registering SPM failed");
        goto out;
    }

    ret_id = sm->id;

out:
    buf[0] = ret_id >> 8;
    buf[1] = ret_id & 0xff;
    link_send_data(buf, sizeof(buf));
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
        "dint\n\t"
        "br   %6\n\t"
        "1:\n\t"
        "mov  &__unprotected_sp, r1\n\t"
        "eint\n\t"
        :
        : "r"(args_left), "m"(ci->arg1), "m"(ci->arg2), "m"(ci->arg3),
          "m"(ci->arg4), "m"(ci->index), "m"(ci->entry)
        : "r6", "r7", "r12", "r13", "r14", "r15");
}

void sm_call(ParseState* state)
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

    CallInfo ci;
    uint16_t* args[] = {&ci.arg1, &ci.arg2, &ci.arg3, &ci.arg4};
    uint8_t* arg_buf = NULL;

    switch (nargs)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            ci.nargs = nargs;

            for (uint16_t i = 0; i < nargs; i++)
            {
                if (!parse_int(state, args[i]))
                {
                    printf("%s: Failed to read argument %u\n", error_prefix, i);
                    return;
                }
            }

            break;

        default:
            if (!parse_raw_data(state, nargs, &arg_buf))
            {
                printf("%s: Failed to read raw arguments\n");
                return;
            }

            ci.nargs = 2;
            ci.arg1 = (uint16_t)arg_buf;
            ci.arg2 = nargs;
            break;
    }

    struct SancusModule* sm = sm_get_by_id(id);

    if (sm == NULL)
    {
        printf("%s: No SM with ID %u\n", id);
        return;
    }

    ci.entry = sm->public_start;
    ci.index = index;

    printf("Accepted call to SM %s, index %u, args %u\n",
           sm->name, index, nargs);

    enter_sm(&ci);
}

void sm_print_identity(ParseState* state)
{
    static const char* error_prefix = "Error reading SmIdentity packet";

    sm_id id;

    if (!parse_int(state, &id))
    {
        printf("%s: Failed to read ID\n");
        return;
    }

    struct SancusModule* sm = sm_get_by_id(id);

    if (sm == NULL)
        return;

    link_send_data(sm->public_start,
                   (char*)sm->public_end - (char*)sm->public_start);

    printf("Identity of SM %s:\n", sm->name);
    print_data(sm->public_start,
               (char*)sm->public_end - (char*)sm->public_start);
    print_data((char*)&sm->public_start, 2);
    print_data((char*)&sm->public_end, 2);
    print_data((char*)&sm->secret_start, 2);
    print_data((char*)&sm->secret_end, 2);
}

struct SancusModule* sm_get_by_id(sm_id id)
{
    SmList* list = find_sm_list(id);
    return list == NULL ? NULL : &list->sm;
}

ElfModule* sm_get_elf_by_id(sm_id id)
{
    SmList* list = find_sm_list(id);
    return list == NULL ? NULL : list->em;
}
