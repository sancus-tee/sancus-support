#ifndef SANCUS_SUPPORT_SPM_CONTROL_H
#define SANCUS_SUPPORT_SPM_CONTROL_H

struct SancusModule;

void sm_load(void);
void sm_call(void);
void sm_print_identity(void);
int  sm_register_existing(struct SancusModule* sm);

#endif

