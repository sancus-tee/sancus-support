#ifndef SANCUS_SUPPORT_TSC_H
#define SANCUS_SUPPORT_TSC_H

#include <stdint.h>

typedef uint64_t tsc_t;

tsc_t tsc_read(void);

#endif
