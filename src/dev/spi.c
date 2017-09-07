#include "spi.h"

#define SPI_DATA   *(volatile uint8_t*)0x0150
#define SPI_CNTRL  *(volatile uint8_t*)0x0151
#define SPI_STATUS *(volatile uint8_t*)0x0152

#define SPI_CPOL            0
#define SPI_CPHA            1
#define SPI_SELECT          2
#define SPI_CLK_DIV         4
#define SPI_SELECT_WIDTH    (SPI_CLK_DIV - SPI_SELECT)

#define SPI_BUSY            0x01

void spi_init(SpiCpol cpol, SpiCpha cpha, unsigned int clk_div)
{
    SPI_CNTRL = (cpol << SPI_CPOL) |
                (cpha << SPI_CPHA) |
                (clk_div << SPI_CLK_DIV);
}

void spi_select(int dev)
{
    dev &= ((1 << SPI_SELECT_WIDTH) - 1);
    SPI_CNTRL |= (dev << SPI_SELECT);
}

uint8_t spi_write_read_byte(uint8_t data)
{
    SPI_DATA = data;
    while (SPI_STATUS & SPI_BUSY) {}
    return SPI_DATA;
}

void spi_write_byte(uint8_t data)
{
    spi_write_read_byte(data);
}

uint8_t spi_read_byte(void)
{
    return spi_write_read_byte(0);
}

void spi_write_read(uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
        data[i] = spi_write_read_byte(data[i]);
}

void spi_write(uint8_t* data, size_t len)
{
    for (size_t i = 0; i < len; i++)
        spi_write_read_byte(data[i]);
}

void spi_read(uint8_t* data, size_t len)
{
    spi_write_read(data, len);
}
