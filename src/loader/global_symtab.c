#include "global_symtab.h"

#include "uart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <spm-support.h>

typedef struct
{
    const char* name;
    void*       value;
} Symbol;

#define SYM(name) {#name, name}

static const Symbol symbols[] =
{
    // libc
    SYM(putchar),
    SYM(puts),
    SYM(printf),
    SYM(strlen),
    SYM(exit),

    // SPM support
    SYM(__unprotected_entry),
    SYM(uart_read),
    SYM(uart_read_byte),
    SYM(uart_write),
    SYM(uart_write_byte)
};

#undef SYM

typedef struct SymbolList
{
    Symbol             symbol;
    ElfModule*         owner;
    struct SymbolList* next;
} SymbolList;

static SymbolList* dynamic_symbols_head = NULL;

void* get_global_symbol_value(const char* name)
{
    unsigned i;
    for (i = 0; i < sizeof(symbols) / sizeof(Symbol); i++)
    {
        if (strcmp(symbols[i].name, name) == 0)
            return symbols[i].value;
    }

    SymbolList* current = dynamic_symbols_head;
    while (current != NULL)
    {
        if (strcmp(current->symbol.name, name) == 0)
            return current->symbol.value;

        current = current->next;
    }

    return UNDEFINED_SYMBOL;
}

int add_global_symbol(const char* name, void* value, ElfModule* owner)
{
    SymbolList** current = &dynamic_symbols_head;
    while (*current != NULL)
        current = &(*current)->next;

    *current = malloc(sizeof(SymbolList));
    if (*current == NULL)
        return 0;

    char* tmp_name = malloc(strlen(name) + 1);
    strcpy(tmp_name, name);
    (*current)->symbol.name = tmp_name;
    (*current)->symbol.value = value;
    (*current)->owner = owner;
    (*current)->next = NULL;
    return 1;
}

void remove_global_symbols(ElfModule* owner)
{
    SymbolList* prev = NULL;
    SymbolList* current = dynamic_symbols_head;
    while (current != NULL)
    {
        if (current->owner == owner)
        {
            if (prev == NULL)
                dynamic_symbols_head = current->next;
            else
                prev->next = current->next;

            SymbolList* next = current->next;
            free((void*)current->symbol.name);
            free(current);
            current = next;
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }
}

static void print_symbol(const Symbol* sym)
{
    printf("%s = %p;\n", sym->name, sym->value);
}

void print_global_symbols(void)
{
    unsigned i;
    for (i = 0; i < sizeof(symbols) / sizeof(Symbol); i++)
        print_symbol(&symbols[i]);

    SymbolList* current = dynamic_symbols_head;
    while (current != NULL)
    {
        print_symbol(&current->symbol);
        current = current->next;
    }

    putchar('\n');
}

