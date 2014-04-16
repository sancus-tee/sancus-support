#ifndef GLOBAL_SYMTAB_H
#define GLOBAL_SYMTAB_H

#include "elf.h"

typedef int (*print_func)(const char*, ...);

int get_global_symbol_value(const char* name, void** dest);
int add_global_symbol(const char* name, void* value, ElfModule* owner);
int add_module_section(const char* name, void* value, ElfModule* owner);
void remove_global_symbols(ElfModule* owner);
void print_global_symbols(print_func pf);
void print_module_sections(ElfModule* module, print_func pf);

#endif

