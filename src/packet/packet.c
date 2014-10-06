#include "packet.h"

#include "private/debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_LEN 512

static LinkApi link;

// write functions *************************************************************
static size_t buf_pos;

static struct __attribute__((packed))
{
    uint8_t header;
    uint8_t buf[BUF_LEN];
} packet;

static size_t buf_available(void)
{
    return sizeof(packet.buf) - buf_pos;
}

static int copy_to_buffer(const uint8_t* data, size_t len)
{
    if (buf_available() < len)
        return 0;

    memcpy(packet.buf + buf_pos, data, len);
    buf_pos += len;
    return 1;
}

static int send_frame(uint8_t* data, size_t len, int finish)
{
    packet.header = finish ? 1 : 0;
    Frame frame;
    frame.data = data;
    frame.len = len;
    return link.send_frame(&frame);
}

static int flush_buffer(int finish)
{
    if (buf_pos == 0)
        return 1;

    if (!send_frame((uint8_t*)&packet, buf_pos + 1, finish))
        return 0;

    buf_pos = 0;
    return 1;
}

int packet_init(LinkApi* api)
{
    link = *api;
    return 1;
}

int packet_start(void)
{
    buf_pos = 0;
    return 1;
}

int packet_write(uint8_t* data, size_t len)
{
    // try to add the data to the buffer
    if (copy_to_buffer(data, len))
        return 1;

    // not enough room in the buffer; flush it and try again
    if (!flush_buffer(/*finish=*/0))
        return 0;

    if (copy_to_buffer(data, len))
        return 1;

    // buffer is too small; send the data directly
    return send_frame(data, len, /*finish=*/0);
}

int packet_finish(void)
{
    return flush_buffer(/*finish=*/1);
}

// queue functions *************************************************************
// we currently only support queueing a single packet
Packet queue = {.len = 0, .data = NULL};

int packet_queue(uint8_t* data, size_t len)
{
    if (queue.data != NULL)
    {
        DBG_PRINTF("Packet queue full, ignoring queue request\n");
        return 0;
    }

    queue.data = malloc(len);

    if (queue.data == NULL)
    {
        DBG_PRINTF("OOM, ignoring queue request\n");
        return 0;
    }

    memcpy(queue.data, data, len);
    queue.len = len;
    return 1;
}

int packet_send_queued(void)
{
    if (queue.data == NULL)
        return 1;

    int result = packet_start() &&
                 packet_write(queue.data, queue.len) &&
                 packet_finish();

    // if anything went wrong, we'll retry next time
    if (result)
    {
        free(queue.data);
        queue.data = NULL;
        queue.len = 0;
    }

    return result;
}

// read functions **************************************************************
// we currently only support single frame packets and packets are not buffered
Packet* received_packet = NULL;

static void receive_packet(void)
{
    DBG_VAR(static const char* oom_msg = "Dropping packet because of OOM");

    if (received_packet != NULL || link.frames_available() == 0)
        return;

    Frame* frame = link.get_next_frame();

    if (frame->len <= 1)
    {
        DBG_PRINTF("Dropping illegal packet\n");
        goto out;
    }

    if (frame->data[0] != 1)
    {
        DBG_PRINTF("Multi-frame packets are not supported yet\n");
        goto out;
    }

    Packet* packet = malloc(sizeof(Packet));

    if (packet == NULL)
    {
        DBG_PRINTF("%s\n", oom_msg);
        goto out;
    }

    packet->len = frame->len - 1;
    packet->data = malloc(packet->len);

    if (packet->data == NULL)
    {
        free(packet);
        DBG_PRINTF("%s\n", oom_msg);
        goto out;
    }

    memcpy(packet->data, frame->data + 1, packet->len);
    received_packet = packet;

out:
    link.free_frame(frame);
}

size_t packet_available(void)
{
    receive_packet();
    return received_packet == NULL ? 0 : 1;
}

Packet* packet_get_next(void)
{
    Packet* ret = received_packet;
    received_packet = NULL;
    return ret;
}

void packet_free(Packet* packet)
{
    free(packet->data);
    free(packet);
}
