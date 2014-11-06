#ifndef SANCUS_SUPPORT_SPI_H
#define SANCUS_SUPPORT_SPI_H

#include <stdlib.h>

typedef enum
{
    SpiCpol0 = 0,
    SpiCpol1 = 1
} SpiCpol;

typedef enum
{
    SpiCpha0 = 0,
    SpiCpha1 = 1
} SpiCpha;

void spi_init(SpiCpol cpol, SpiCpha cpha, unsigned int clk_div);
void spi_select(void);
void spi_deselect(void);

uint8_t spi_write_read_byte(uint8_t data);
void spi_write_byte(uint8_t data);
uint8_t spi_read_byte(void);

/// The @p data buffer is overwritten with the read data.
void spi_write_read(uint8_t* data, size_t len);
void spi_write(uint8_t* data, size_t len);
void spi_read(uint8_t* data, size_t len);

#endif
