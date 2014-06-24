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
} Frame;

int link_init(PhyApi* phy_api);
size_t link_frames_available(void);
Frame* link_get_next_frame(void);
int link_send_frame(Frame* frame);
int link_send_data(uint8_t* data, size_t len);
int link_send_byte(uint8_t byte);
void link_free_frame(Frame* frame);

#endif
