#include "link.h"

#include "cobs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INIT_PACKET_LEN 16

typedef struct PacketList
{
    Packet*            packet;
    struct PacketList* next;
} PacketList;

static PhyApi phy = {NULL, NULL, NULL};
static PacketList* packets_head = NULL;
static uint8_t* buffer = NULL;
static size_t buffer_len = 0;
static size_t buffer_pos = 0;

int link_init(PhyApi* phy_api)
{
    phy = *phy_api;

    puts("Link layer initialized using the following PHY API:");
    printf(" - available(): %p\n", phy.available);
    printf(" - read():      %p\n", phy.read);
    printf(" - write():     %p\n", phy.write);

    return 1;
}

// returns 1 iff the last flushed byte was a 0 byte
static int flush_until_null_byte(void)
{
    while (phy.available() > 0)
    {
        if (phy.read() == 0)
            return 1;
    }

    return 0;
}

// returns 1 if the start of a new packet has been found (that is, the next byte
// from PHY is the first byte in the packet) and 0 otherwise (no more bytes
// available from PHY before the start of a packet was found or some error)
static int start_new_packet(void)
{
    if (!flush_until_null_byte())
        return 0;

    buffer = malloc(INIT_PACKET_LEN);
    buffer_len = INIT_PACKET_LEN;
    buffer_pos = 0;

    if (buffer == NULL)
    {
        buffer_len = 0;
        return 0;
    }

    return 1;
}

static int realloc_buffer(void)
{
    size_t new_len = buffer_len * 2;
    u_int8_t* new_buf = malloc(new_len);

    if (new_buf == NULL)
        return 0;

    memcpy(new_buf, buffer, buffer_len);
    free(buffer);
    buffer = new_buf;
    buffer_len = new_len;
    return 1;
}

static void free_buffer(void)
{
    free(buffer);
    buffer = NULL;
    buffer_len = 0;
    buffer_pos = 0;
}

// return 1 iff the end of the current packet has been found
static int continue_packet(void)
{
    while (phy.available() > 0)
    {
        if (buffer_pos == buffer_len)
        {
            if (!realloc_buffer())
                return 0;
        }

        uint8_t byte = phy.read();

        if (byte == 0)
            return 1;

        buffer[buffer_pos++] = byte;
    }

    return 0;
}

static int has_partial_packet(void)
{
    return buffer != NULL;
}

static Packet* new_packet(size_t len)
{
    Packet* packet = malloc(sizeof(Packet));

    if (packet == NULL)
        return NULL;

    packet->len = len;
    packet->data = malloc(len);

    if (packet->data == NULL)
    {
        free(packet);
        return NULL;
    }

    return packet;
}

static int enqueue_packet(Packet* packet)
{
    PacketList** current = &packets_head;

    while (*current != NULL)
        current = &(*current)->next;

    PacketList* new_element = malloc(sizeof(PacketList));

    if (new_element == NULL)
        return 0;

    new_element->packet = packet;
    new_element->next = NULL;
    *current = new_element;
    return 1;
}

static Packet* dequeue_packet(void)
{
    if (packets_head == NULL)
        return NULL;

    PacketList* head = packets_head;
    Packet* packet = head->packet;
    packets_head = head->next;
    free(head);
    return packet;
}

static int finish_packet(void)
{
    Packet* packet = new_packet(buffer_pos);

    if (packet == NULL)
        return 0;

    if (!cobs_decode(buffer, buffer_pos, packet->data, &packet->len))
    {
        puts("Dropping malformed packet");
        goto error;
    }

    if (packet->len == 0)
    {
        puts("Dropping zero-length packet");
        goto error;
    }

    if (!enqueue_packet(packet))
    {
        puts("Dropping packet due to memory pressure");
        goto error;
    }

    free_buffer();
    return 1;

error:
    free_buffer();
    link_free_packet(packet);
    return 0;
}

static void read_phy(void)
{
    if (!phy.available())
        return;

    if (!has_partial_packet())
    {
        if (!start_new_packet())
            return;
    }

    if (continue_packet())
        finish_packet();
}

static size_t packet_count(void)
{
    size_t count = 0;
    PacketList** current = &packets_head;

    while (*current != NULL)
    {
        count++;
        current = &(*current)->next;
    }

    return count;
}

size_t link_packets_available(void)
{
    read_phy();
    return packet_count();
}

Packet* link_get_next_packet(void)
{
    read_phy();
    return dequeue_packet();
}

int link_send_packet(Packet* packet)
{
    uint8_t* buf = malloc(cobs_max_encoded_len(packet->len));

    if (buf == NULL)
        return 0;

    size_t len;
    if (!cobs_encode(packet->data, packet->len, buf, &len))
    {
        free(buf);
        return 0;
    }

    phy.write(0x00);

    size_t i;
    for (i = 0; i < len; i++)
        phy.write(buf[i]);

    phy.write(0x00);

    free(buf);
    return 1;
}

int link_send_data(uint8_t* data, size_t len)
{
    Packet packet = {data, len};
    return link_send_packet(&packet);
}

void link_free_packet(Packet* packet)
{
    free(packet->data);
    free(packet);
}
