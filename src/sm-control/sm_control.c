#include "sm_control.h"

#include "elf.h"
#include "global_symtab.h"
#include "private/debug.h"

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
        DBG_PRINTF("No SM with ID 0x%x\n", id);
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

static struct SancusModule* register_sm(const char* name, uint16_t vendor_id,
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
        DBG_PRINTF("Failed to find SM symbols\n");
        free(sm_list);
        return NULL;
    }

#ifdef NO_PROTECT
    sm->id = next_id++;
#else
    if (!sancus_enable(sm))
    {
        DBG_PRINTF("Protecting SM failed\n");
        free(sm_list);
        return NULL;
    }
#endif

    append_sm_list(sm_list);

    DBG_PRINTF("Registered SM %s with id 0x%x for vendor 0x%x\n",
               name, sm->id, vendor_id);
    DBG_PRINTF(" - Public: [%p, %p]\n", sm->public_start, sm->public_end);
    DBG_PRINTF(" - Secret: [%p, %p]\n", sm->secret_start, sm->secret_end);
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

sm_id sm_load(void* file, const char* name, vendor_id vid)
{
    ElfModule* em = elf_load(file);

    if (em == NULL)
    {
        DBG_PRINTF("Error loading module\n");
        return 0;
    }

    DBG_PRINTF("Module successfully loaded\n");

    struct SancusModule* sm = register_sm(name, vid, em);

    if (sm == NULL)
    {
        DBG_PRINTF("Registering SPM failed\n");
        return 0;
    }

    DBG_PRINTF("Module successfully registered\n");
    return sm->id;
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

// if this function is inlined, GCC messes with the stack pointer
// if this function is not optimized, the register allocator fails
static uint16_t __attribute__((noinline, optimize("-O1")))
enter_sm(CallInfo* ci)
{
    uint16_t args_left = ci->nargs;
    uint16_t ret;

    asm("tst  %1\n\t"
        "jz   1f\n\t"
        "mov  %2, r15\n\t"
        "dec  %1\n\t"
        "jz   1f\n\t"
        "mov  %3, r14\n\t"
        "dec  %1\n\t"
        "jz   1f\n\t"
        "mov  %4, r13\n\t"
        "dec  %1\n\t"
        "jz   1f\n\t"
        "mov  %5, r12\n\t"
        "dec  %1\n\t"

        "1:\n\t"
        "push r2\n\t"
        "mov  r1, &__unprotected_sp\n\t"
        "mov  %6, r6\n\t"
        "mov  #1f, r7\n\t"
        "dint\n\t"
        "br   %7\n\t"
        "1:\n\t"
        "mov  &__unprotected_sp, r1\n\t"
        "pop r2\n\t"
        "mov r15, %0\n\t"
        : "=m"(ret)
        : "r"(args_left), "m"(ci->arg1), "m"(ci->arg2), "m"(ci->arg3),
          "m"(ci->arg4), "m"(ci->index), "m"(ci->entry)
        : "r6", "r7", "r12", "r13", "r14", "r15");

    return ret;
}

int sm_call_module(struct SancusModule* sm, uint16_t index,
                   uint16_t* args, size_t nargs, uint16_t* retval)
{
    CallInfo ci;
    ci.entry = sm->public_start;
    ci.index = index;
    ci.nargs = nargs;

    switch (nargs)
    {
        case 4:
            ci.arg4 = args[3];
        case 3:
            ci.arg3 = args[2];
        case 2:
            ci.arg2 = args[1];
        case 1:
            ci.arg1 = args[0];
        case 0:
            break;
        default:
            DBG_PRINTF("Illegal number of arguments: %u\n", nargs);
            return 0;
    }

    DBG_PRINTF("Accepted call to SM %s, index %u, args %u\n",
               sm->name, index, nargs);

    uint16_t ret = enter_sm(&ci);

    if (retval != NULL)
        *retval = ret;

    return 1;
}

int sm_call_id(sm_id id, uint16_t index, uint16_t* args, size_t nargs,
               uint16_t* retval)
{
    struct SancusModule* sm = sm_get_by_id(id);

    if (sm == NULL)
    {
        DBG_PRINTF("No SM with ID %u\n", id);
        return 0;
    }

    return sm_call_module(sm, index, args, nargs, retval);
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
