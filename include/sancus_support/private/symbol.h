#ifndef PRIVATE_SYMBOL_H
#define PRIVATE_SYMBOL_H

typedef struct
{
    const char* name;
    void*       value;
    int         is_section;
} Symbol;

#endif
