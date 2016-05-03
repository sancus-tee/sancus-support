#ifndef SANCUS_SUPPORT_PMODCLS_H
#define SANCUS_SUPPORT_PMODCLS_H

typedef enum
{
    PmodClsWrapAt16,
    PmodClsWrapAt40
} PmodClsWrapMode;

void pmodcls_init(void);
int  pmodcls_putchar(int c);
void pmodcls_write(const char* text);
void pmodcls_clear(void);
void pmodcls_set_wrap_mode(PmodClsWrapMode mode);

#endif
