#include "spm_control.h"

#include "uart.h"
#include "elf.h"
#include "tools.h"
#include "global_symtab.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <spm-support.h>

typedef struct SpmList
{
    struct Spm      spm;
    ElfModule*      em;
    struct SpmList* next;
} SpmList;

SpmList* spm_head = NULL;

static void* get_spm_symbol(char* spm_name, char* which)
{
    char* name = malloc(strlen("__spm_") + strlen(spm_name) +
                        strlen(which) + 2);
    if (name == NULL)
        return NULL;

    strcpy(name, "__spm_");
    strcat(name, spm_name);
    strcat(name, "_");
    strcat(name, which);
    void* sym = get_global_symbol_value(name);
    free(name);
    return sym;
}

static int register_spm(char* name, uint16_t vendor_id, ElfModule* em)
{
    SpmList** current = &spm_head;
    while (*current != NULL)
        current = &(*current)->next;

    *current = malloc(sizeof(SpmList));
    if (*current == NULL)
        return 0;

    (*current)->em = em;
    (*current)->next = NULL;

    struct Spm* spm = &(*current)->spm;
    spm->id = 0;
    spm->vendor_id = vendor_id;
    spm->name = name;
    spm->public_start = get_spm_symbol(name, "public_start");
    spm->public_end = get_spm_symbol(name, "public_end");
    spm->secret_start = get_spm_symbol(name, "secret_start");
    spm->secret_end = get_spm_symbol(name, "secret_end");

    if (spm->public_start == NULL || spm->public_end == NULL ||
        spm->secret_start == NULL || spm->secret_end == NULL)
    {
        puts("Failed to find SPM symbols");
        free(*current);
        *current = NULL;
        return 0;
    }

    if (!protect_spm(spm))
    {
        puts("Protecting SPM failed");
        free(*current);
        *current = NULL;
        return 0;
    }

    printf("Registered SPM %s with id 0x%x for vendor 0x%x\n",
           name, spm->id, vendor_id);
    printf(" - Public: [%p, %p]\n", spm->public_start, spm->public_end);
    printf(" - Secret: [%p, %p]\n", spm->secret_start, spm->secret_end);
    return 1;
}

void spm_load(void)
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
    unsigned i;
    for (i = 0; i < size; i++)
    {
        printf("%02x ", file[i]);
        if ((i + 1) % 26 == 0)
            printf("\n");
    }

    printf("\nLoading...\n");

    ElfModule* em = elf_load(file);
    free(file);

    if (em == NULL)
    {
        puts("Error loading module");
        return;
    }

    puts("Module successfully loaded");

    if (!register_spm(name, vendor_id, em))
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
static void __attribute__((optimize("-O1"))) enter_spm(CallInfo* ci)
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

void spm_call(void)
{
    uint16_t id = read_int();
    uint16_t index = read_int();
    uint16_t nargs = read_int();

    SpmList* current = spm_head;
    while (current != NULL && current->spm.id != id)
        current = current->next;

    if (current == NULL)
    {
        printf("No SPM with ID 0x%x\n", id);
        return;
    }

    CallInfo ci;
    ci.entry = current->spm.public_start;
    ci.index = index;
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

    printf("Accepted call to SPM %s, index %u, args %u\n",
           current->spm.name, index, nargs);

    enter_spm(&ci);
    free(arg_buf);
}

