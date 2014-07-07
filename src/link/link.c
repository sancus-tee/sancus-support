#include "link.h"

#include "cobs.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INIT_FRAME_LEN 16

typedef struct FrameList
{
    Frame*            frame;
    struct FrameList* next;
} FrameList;

static PhyApi phy = {NULL, NULL, NULL};
static FrameList* frames_head = NULL;
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

// returns 1 if the start of a new frame has been found (that is, the next byte
// from PHY is the first byte in the frame) and 0 otherwise (no more bytes
// available from PHY before the start of a frame was found or some error)
static int start_new_frame(void)
{
    if (!flush_until_null_byte())
        return 0;

    buffer = malloc(INIT_FRAME_LEN);
    buffer_len = INIT_FRAME_LEN;
    buffer_pos = 0;

    if (buffer == NULL)
    {
        puts("link OOM: start_new_frame");
        buffer_len = 0;
        return 0;
    }

    return 1;
}

static int realloc_buffer(void)
{
    // buffer size is increased exponentially until 4096B after which it is
    // increased linearly. this is because allocations of 4K generally succeed
    // while those of 8K don't.
    size_t new_len = buffer_len < 4096 ? buffer_len * 2 : buffer_len + 512;
    u_int8_t* new_buf = malloc(new_len);

    if (new_buf == NULL)
    {
        printf("link OOM: realloc_buffer(%u); discarding frame\n", new_len);
        free(buffer);
        buffer_len = 0;
        buffer_pos = 0;
        return 0;
    }

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

// return 1 iff the end of the current frame has been found
static int continue_frame(void)
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
        {
            // ignore null bytes at the beginning of a frame
            if (buffer_pos == 0)
                continue;
            else
                return 1;
        }

        buffer[buffer_pos++] = byte;
    }

    return 0;
}

static int has_partial_frame(void)
{
    return buffer != NULL;
}

static Frame* new_frame(size_t len)
{
    Frame* frame = malloc(sizeof(Frame));

    if (frame == NULL)
        return NULL;

    frame->len = len;
    frame->data = malloc(len);

    if (frame->data == NULL)
    {
        free(frame);
        return NULL;
    }

    return frame;
}

static int enqueue_frame(Frame* frame)
{
    FrameList** current = &frames_head;

    while (*current != NULL)
        current = &(*current)->next;

    FrameList* new_element = malloc(sizeof(FrameList));

    if (new_element == NULL)
        return 0;

    new_element->frame = frame;
    new_element->next = NULL;
    *current = new_element;
    return 1;
}

static Frame* dequeue_frame(void)
{
    if (frames_head == NULL)
        return NULL;

    FrameList* head = frames_head;
    Frame* frame = head->frame;
    frames_head = head->next;
    free(head);
    return frame;
}

static int finish_frame(void)
{
    Frame* frame = new_frame(buffer_pos);

    if (frame == NULL)
        return 0;

    if (!cobs_decode(buffer, buffer_pos, frame->data, &frame->len))
    {
        puts("Dropping malformed frame");
        goto error;
    }

    if (frame->len == 0)
    {
        puts("Dropping zero-length frame");
        goto error;
    }

    if (!enqueue_frame(frame))
    {
        puts("Dropping frame due to memory pressure");
        goto error;
    }

    free_buffer();
    return 1;

error:
    free_buffer();
    link_free_frame(frame);
    return 0;
}

static void read_phy(void)
{
    if (!phy.available())
        return;

    if (!has_partial_frame())
    {
        if (!start_new_frame())
            return;
    }

    if (continue_frame())
        finish_frame();
}

static size_t frame_count(void)
{
    size_t count = 0;
    FrameList** current = &frames_head;

    while (*current != NULL)
    {
        count++;
        current = &(*current)->next;
    }

    return count;
}

size_t link_frames_available(void)
{
    read_phy();
    return frame_count();
}

Frame* link_get_next_frame(void)
{
    read_phy();
    return dequeue_frame();
}

int link_send_frame(Frame* frame)
{
    printf("Sending %u byte frame\n", frame->len);
    uint8_t* buf = malloc(cobs_max_encoded_len(frame->len));

    if (buf == NULL)
    {
        puts("Out of memory, dropping outgoing frame");
        return 0;
    }

    size_t len;
    if (!cobs_encode(frame->data, frame->len, buf, &len))
    {
        free(buf);
        return 0;
    }

    printf("Frame encoded in %u + 2 bytes\n", len);

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
    Frame frame = {data, len};
    return link_send_frame(&frame);
}

int link_send_byte(uint8_t byte)
{
    Frame frame = {&byte, 1};
    return link_send_frame(&frame);
}

void link_free_frame(Frame* frame)
{
    free(frame->data);
    free(frame );
}
