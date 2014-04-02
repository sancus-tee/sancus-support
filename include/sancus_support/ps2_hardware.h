#ifndef PS2_HARDWARE_H
#define PS2_HARDWARE_H

#define PS2_STAT (*(volatile unsigned char*) 0x00a0)
#define PS2_DATA (*(volatile unsigned char*) 0x00a1)

#define PS2_READY    0x01
#define PS2_BUSY     0x02
#define PS2_ERROR    0x04
#define PS2_RELEASED 0x08
#define PS2_EXTENDED 0x10

#define PS2_RX_VECTOR (5 * 2)

#endif

