#ifndef SANCUS_SUPPORT_SPM_CONTROL_H
#define SANCUS_SUPPORT_SPM_CONTROL_H

#include <stdint.h>

#include <sancus/sm_support.h>

struct ElfModule;
struct ParseState;

sm_id sm_load(void* file, const char* name, vendor_id vid);
int sm_call_id(sm_id id, uint16_t index, uint16_t* args, size_t nargs);
int sm_call_module(struct SancusModule* sm, uint16_t index,
                   uint16_t* args, size_t nargs);
int sm_register_existing(struct SancusModule* sm);
struct SancusModule* sm_get_by_id(sm_id id);
struct ElfModule* sm_get_elf_by_id(sm_id id);

#endif
