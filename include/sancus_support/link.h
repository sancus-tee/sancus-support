#ifndef SANCUS_SUPPORT_LINK_H
#define SANCUS_SUPPORT_LINK_H

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t (*read)(void);
    void    (*write)(uint8_t);
    size_t  (*available)(void);
} PhyApi;

typedef struct
{
    uint8_t* data;
    size_t   len;
} Packet;

int link_init(PhyApi* phy_api);
size_t link_packets_available(void);
Packet* link_get_next_packet(void);
int link_send_packet(Packet* packet);
int link_send_data(uint8_t* data, size_t len);
int link_send_byte(uint8_t byte);
void link_free_packet(Packet* packet);

#endif
