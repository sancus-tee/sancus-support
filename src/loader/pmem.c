#include "pmem.h"

#define HEAP_END 0xffe0

extern char _etext;
extern char __data_end_rom;
static uintptr_t heap = (uintptr_t)&__data_end_rom;

// TODO implement something that isn't completely ridiculous :-)
void* pmem_malloc(size_t size)
{
    uintptr_t new_heap = heap + size;

    // the second test detect integer overflow
    if (new_heap > HEAP_END || new_heap < size)
        return NULL;

    void* ret = (void*)heap;
    heap += size;

    // make sure the heap stays aligned
    if (heap % 2 == 1)
        heap++;

    return ret;
}

void pmem_free(void* ptr)
{
}

