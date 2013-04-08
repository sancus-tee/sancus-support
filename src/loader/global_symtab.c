#include "global_symtab.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
    const char* name;
    void*       value;
} Symbol;

static const Symbol symbols[] =
{
    {"puts",   puts},
    {"printf", printf}
};

void* get_global_symbol_value(const char* name)
{
    unsigned i;
    for (i = 0; i < sizeof(symbols) / sizeof(Symbol); i++)
    {
        if (strcmp(symbols[i].name, name) == 0)
            return symbols[i].value;
    }

    return NULL;
}

