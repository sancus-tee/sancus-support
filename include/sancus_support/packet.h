#ifndef SANCUS_SUPPORT_PACKET_H
#define SANCUS_SUPPORT_PACKET_H

#include "link.h"

typedef struct
{
    size_t (*frames_available)(void);
    Frame* (*get_next_frame)(void);
    void (*free_frame)(Frame*);
    int (*send_frame)(Frame*);
} LinkApi;

typedef struct
{
    size_t len;
    uint8_t* data;
} Packet;

int packet_init(LinkApi* api);

int packet_start(void);
int packet_write(uint8_t* data, size_t len);
int packet_finish(void);

size_t packet_available(void);
Packet* packet_get_next(void);
void packet_free(Packet* packet);

#endif
