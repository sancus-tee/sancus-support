#include "cobs.h"

int cobs_encode(const uint8_t* src, size_t len,
                uint8_t* dst, size_t* encoded_len)
{
    const uint8_t* const src_end = src + len;
    uint8_t* const dst_begin = dst;
    uint8_t* code_ptr = dst++;
    *code_ptr = 1;

    while (src < src_end)
    {
        if (*src == 0)
        {
            // 0 byte: finish current block without writing the 0
            code_ptr = dst++;
            *code_ptr = 1;
            src++;
        }
        else
        {
            // normal byte: copy and increase count
            *dst++ = *src++;
            (*code_ptr)++;

            if (*code_ptr == 0xff)
            {
                // start a new block if the current block is full
                code_ptr = dst++;
                *code_ptr = 1;
            }
        }
    }

    *encoded_len = dst - dst_begin;
    return 1;
}

int cobs_decode(const uint8_t* src, size_t len,
                uint8_t* dst, size_t* decoded_len)
{
    const uint8_t* const src_end = src + len;
    uint8_t* const dst_begin = dst;

    while (src < src_end)
    {
        uint8_t code = *src;

        // sanity checks for illegal code: 0 should never happen and we also
        // check if code is not too large, preventing heartbleed-like bugs :-)
        if (code == 0 || src + code > src_end)
            return 0;

        src++;

        uint8_t i;
        for (i = 1; i < code; i++)
        {
            // a 0 byte should never occur in the input
            if (*src == 0)
                return 0;

            *dst++ = *src++;
        }

        if (code != 0xff)
            *dst++ = 0;
    }

    uint8_t* dst_end;

    if (dst > dst_begin && *(dst - 1) == 0) // drop last 0 byte
        dst_end = dst - 1;
    else
        dst_end = dst;

    *decoded_len = dst_end - dst_begin;
    return 1;
}

size_t cobs_max_encoded_len(size_t len)
{
    return len + len / 254 + 1;
}
