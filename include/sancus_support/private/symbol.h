#ifndef PRIVATE_SYMBOL_H
#define PRIVATE_SYMBOL_H

typedef struct
{
    const char* name;
    void*       value;
} Symbol;

#define STATIC_SYMBOLS_START const Symbol static_symbols[] = {
#define SYMBOL(name) {#name, &name}
#define STATIC_SYMBOLS_END \
    }; \
    const unsigned num_static_symbols = sizeof(static_symbols) / sizeof(Symbol);

#endif
