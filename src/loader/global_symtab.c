#include "global_symtab.h"
#include "private/symbol.h"

#include <string.h>
#include <stdlib.h>

// these weak symbols will be replaced if the automatic symtab generation script
// is used
Symbol __attribute__((weak)) static_symbols[] = {};
size_t __attribute__((weak)) num_static_symbols = 0;

typedef struct SymbolList
{
    Symbol             symbol;
    int                is_section;
    ElfModule*         owner;
    struct SymbolList* next;
} SymbolList;

static SymbolList* dynamic_symbols_head = NULL;

static int symbol_matches(const Symbol* sym, const char* name)
{
    return strcmp(sym->name, name) == 0;
}

size_t symtab_get_num_symbols()
{
    size_t num_dyns = 0;
    SymbolList* current = dynamic_symbols_head;

    while (current != NULL)
    {
        num_dyns++;
        current = current->next;
    }

    return num_dyns + num_static_symbols;
}

int symtab_get_symbol(size_t i, Symbol* sym, int* is_section, ElfModule** owner)
{
    if (i < num_static_symbols)
    {
        *sym = static_symbols[i];
        *is_section = 0;
        *owner = NULL;
        return 1;
    }

    i -= num_static_symbols;
    SymbolList* current = dynamic_symbols_head;

    while (current != NULL && i != 0)
    {
        current = current->next;
        i--;
    }

    if (current == NULL)
        return 0;

    *sym = current->symbol;
    *is_section = current->is_section;
    *owner = current->owner;
    return 1;
}

int get_global_symbol_value(const char* name, void** dest)
{
    unsigned i;
    for (i = 0; i < num_static_symbols; i++)
    {
        const Symbol* sym = &static_symbols[i];
        if (symbol_matches(sym, name))
        {
            *dest = sym->value;
            return 1;
        }
    }

    SymbolList* current = dynamic_symbols_head;
    while (current != NULL)
    {
        if (!current->is_section)
        {
            const Symbol* sym = &current->symbol;
            if (symbol_matches(sym, name))
            {
                *dest = sym->value;
                return 1;
            }
        }

        current = current->next;
    }

    return 0;
}

static int add_symbol(const char* name, void* value, ElfModule* owner,
                      int is_section)
{
    SymbolList** current = &dynamic_symbols_head;
    while (*current != NULL)
        current = &(*current)->next;

    *current = malloc(sizeof(SymbolList));
    if (*current == NULL)
        return 0;

    char* tmp_name = strdup(name);
    if (tmp_name == NULL)
    {
        free(current);
        return 0;
    }

    (*current)->symbol.name = tmp_name;
    (*current)->symbol.value = value;
    (*current)->is_section = is_section;
    (*current)->owner = owner;
    (*current)->next = NULL;
    return 1;
}

int add_global_symbol(const char* name, void* value, ElfModule* owner)
{
    return add_symbol(name, value, owner, /*is_section=*/0);
}

int add_module_section(const char* name, void* value, ElfModule* owner)
{
    return add_symbol(name, value, owner, /*is_section=*/1);
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
