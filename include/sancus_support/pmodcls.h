#ifndef SANCUS_SUPPORT_PMODCLS_H
#define SANCUS_SUPPORT_PMODCLS_H

typedef enum
{
    PmodClsWrapAt16,
    PmodClsWrapAt40
} PmodClsWrapMode;

typedef char pmodcls_pos_t;

void pmodcls_init(void);
int  pmodcls_putchar(int c);
void pmodcls_write(const char* text);
void pmodcls_clear(void);
void pmodcls_set_wrap_mode(PmodClsWrapMode mode);
void pmodcls_set_cursor_position(pmodcls_pos_t row, pmodcls_pos_t col);

#endif
