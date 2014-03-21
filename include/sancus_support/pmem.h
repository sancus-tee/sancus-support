#ifndef SANCUS_SUPPORT_PMEM_H
#define SANCUS_SUPPORT_PMEM_H

#include <stdlib.h>

void* pmem_malloc(size_t size);
void  pmem_free(void* ptr);

#endif

