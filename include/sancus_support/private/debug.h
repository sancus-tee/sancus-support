#ifndef PRIVATE_DEBUG_H
#define PRIVATE_DEBUG_H

#ifndef NDEBUG
#include <stdio.h>
#define DBG_PRINTF(...) printf(__VA_ARGS__)
#define DBG_VAR(...) __VA_ARGS__
#else
#define DBG_PRINTF(...)
#define DBG_VAR(...)
#endif

#endif
