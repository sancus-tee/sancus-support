#ifndef MSP_SUPPORT_ELF_H
#define MSP_SUPPORT_ELF_H

#include <stddef.h>

typedef struct ElfModule ElfModule;


ElfModule* elf_load(void* file);
void elf_unload(ElfModule* em);

#endif

