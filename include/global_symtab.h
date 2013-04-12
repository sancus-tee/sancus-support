#ifndef GLOBAL_SYMTAB_H
#define GLOBAL_SYMTAB_H

#include "elf.h"

#define UNDEFINED_SYMBOL (void*)0xffff

void* get_global_symbol_value(const char* name);
int add_global_symbol(const char* name, void* value, ElfModule* owner);
void remove_global_symbols(ElfModule* owner);
void print_global_symbols(void);

#endif

