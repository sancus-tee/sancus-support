#ifndef SANCUS_SUPPORT_ELF_H
#define SANCUS_SUPPORT_ELF_H

#include <stddef.h>

typedef struct ElfModule ElfModule;

ElfModule* elf_load(void* file);
void elf_unload(ElfModule* em);

#endif

