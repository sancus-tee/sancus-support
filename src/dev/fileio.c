#include "fileio.h"

#define FILEIO_STATUS (*(volatile unsigned char*)0x00c0)
#define FILEIO_DATA   (*(volatile unsigned char*)0x00c2)

#define FILEIO_AVAILABLE 0x01

int fileio_available(void)
{
    return FILEIO_STATUS & FILEIO_AVAILABLE;
}

unsigned char fileio_getc(void)
{
    return FILEIO_DATA;
}

void fileio_putc(unsigned char c)
{
    FILEIO_DATA = c;
}
