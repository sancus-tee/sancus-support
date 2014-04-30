#ifndef SANCUS_SUPPORT_COBS_H
#define SANCUS_SUPPORT_COBS_H

#include <stdint.h>
#include <stddef.h>

// dst should be at least cobs_max_encoded_len(len) long
int cobs_encode(const uint8_t* src, size_t len,
                uint8_t* dst, size_t* encoded_len);

// dst should be at least len long
int cobs_decode(const uint8_t* src, size_t len,
                uint8_t* dst, size_t* decoded_len);

size_t cobs_max_encoded_len(size_t len);

#endif
