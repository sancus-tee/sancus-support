#ifndef MSP_SUPPORT_PMEM_H
#define MSP_SUPPORT_PMEM_H

#include <stdlib.h>

void* pmem_malloc(size_t size);
void  pmem_free(void* ptr);

#endif

