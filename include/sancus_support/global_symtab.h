#ifndef GLOBAL_SYMTAB_H
#define GLOBAL_SYMTAB_H

#include "elf.h"

#include "private/symbol.h"

int get_global_symbol_value(const char* name, void** dest);
int add_global_symbol(const char* name, void* value, ElfModule* owner);
int add_module_section(const char* name, void* value, ElfModule* owner);
void remove_global_symbols(ElfModule* owner);
size_t symtab_get_num_symbols(void);
int symtab_get_symbol(size_t i, Symbol* sym, int* is_section,
                      ElfModule** owner);

#endif

