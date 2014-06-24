#ifndef SANCUS_SUPPORT_SPM_CONTROL_H
#define SANCUS_SUPPORT_SPM_CONTROL_H

#include <sancus/sm_support.h>

struct ElfModule;
struct ParseState;

void sm_load(struct ParseState* state);
void sm_call(struct ParseState* state);
void sm_print_identity(struct ParseState* state);
int sm_register_existing(struct SancusModule* sm);
struct SancusModule* sm_get_by_id(sm_id id);
struct ElfModule* sm_get_elf_by_id(sm_id id);

#endif

