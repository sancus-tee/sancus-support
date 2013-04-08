#ifndef MSP_SUPPORT_ELF_H
#define MSP_SUPPORT_ELF_H

#include <stddef.h>

typedef struct
{
    void* pmem;
    void* dmem;
} ElfModule;

int elf_load(void* file, size_t size, ElfModule* em);
void elf_unload(ElfModule* em);

#endif

