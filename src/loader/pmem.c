#include "pmem.h"

#define HEAP_END 0xffe0

extern char _etext;
extern char __data_end_rom;
static uintptr_t heap = (uintptr_t)&__data_end_rom;

// TODO implement something that isn't completely ridiculous :-)
void* pmem_malloc(size_t size)
{
    if (heap + size > HEAP_END)
        return NULL;

    void* ret = (void*)heap;
    heap += size;
    return ret;
}

void pmem_free(void* ptr)
{
}

