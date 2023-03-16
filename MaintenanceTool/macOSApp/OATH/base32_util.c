//
//  base32_util.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/16.
//
#include <string.h>
#include "base32_util.h"

static uint8_t decoding_table[256] = {
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0x00 - 0x0F
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0x10 - 0x1F
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0x20 - 0x2F
    255,255, 26, 27,  28, 29, 30, 31, 255,255,255,255, 255,  0,255,255,  // 0x30 - 0x3F
    255,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,  // 0x40 - 0x4F
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,255, 255,255,255,255,  // 0x50 - 0x5F
    255,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,  // 0x60 - 0x6F
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,255, 255,255,255,255,  // 0x70 - 0x7F
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0x80 - 0x8F
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0x90 - 0x9F
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xA0 - 0xAF
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xB0 - 0xBF
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xC0 - 0xCF
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xD0 - 0xDF
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xE0 - 0xEF
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,  // 0xF0 - 0xFF
};

static uint8_t padding_adjustment[8] = {
    0, 1, 1, 1, 2, 3, 3, 4
};

bool base32_decode(uint8_t *decoded, size_t *decoded_size, const char *encoded)
{
    // uint8_t overflow check
    uint8_t encoded_size = strlen(encoded);
    if (encoded_size >= (255 - 7)) {
        return false;
    }
    uint8_t encoded_blocks = (encoded_size + 7) >> 3;
    // uint8_t expected_size = encoded_blocks * 5;

    // Destination area check
    uint8_t *decoded_bytes = decoded;
    if (decoded_bytes == NULL && decoded_size == 0) {
        return false;
    }

    uint8_t encoded_byte1, encoded_byte2, encoded_byte3, encoded_byte4;
    uint8_t encoded_byte5, encoded_byte6, encoded_byte7, encoded_byte8;
    
    uint8_t encoded_base_idx  = 0;
    uint8_t decoded_base_idx  = 0;
    uint8_t encoded_block[8]  = {0,0,0,0,0,0,0,0};
    uint8_t encoded_block_idx = 0;
    uint8_t c;
    for (uint8_t encoded_bytes_to_process = encoded_size; encoded_bytes_to_process > 0; encoded_bytes_to_process--) {
        c = encoded[encoded_base_idx++];
        if (c == '=') {
            // padding...
            break;
        }
        c = decoding_table[c];
        if (c == 255) {
            continue;
        }

        encoded_block[encoded_block_idx++] = c;
        if (encoded_block_idx == 8) {
            encoded_byte1 = encoded_block[0];
            encoded_byte2 = encoded_block[1];
            encoded_byte3 = encoded_block[2];
            encoded_byte4 = encoded_block[3];
            encoded_byte5 = encoded_block[4];
            encoded_byte6 = encoded_block[5];
            encoded_byte7 = encoded_block[6];
            encoded_byte8 = encoded_block[7];
            decoded_bytes[decoded_base_idx]   = ((encoded_byte1 << 3) & 0xF8) | ((encoded_byte2 >> 2) & 0x07);
            decoded_bytes[decoded_base_idx+1] = ((encoded_byte2 << 6) & 0xC0) | ((encoded_byte3 << 1) & 0x3E) | ((encoded_byte4 >> 4) & 0x01);
            decoded_bytes[decoded_base_idx+2] = ((encoded_byte4 << 4) & 0xF0) | ((encoded_byte5 >> 1) & 0x0F);
            decoded_bytes[decoded_base_idx+3] = ((encoded_byte5 << 7) & 0x80) | ((encoded_byte6 << 2) & 0x7C) | ((encoded_byte7 >> 3) & 0x03);
            decoded_bytes[decoded_base_idx+4] = ((encoded_byte7 << 5) & 0xE0) |  (encoded_byte8 & 0x1F);
            decoded_base_idx += 5;
            encoded_block_idx = 0;
        }
    }
    encoded_byte7 = 0;
    encoded_byte6 = 0;
    encoded_byte5 = 0;
    encoded_byte4 = 0;
    encoded_byte3 = 0;
    encoded_byte2 = 0;
    switch (encoded_block_idx) {
        case 7:
            encoded_byte7 = encoded_block[6];
        case 6:
            encoded_byte6 = encoded_block[5];
        case 5:
            encoded_byte5 = encoded_block[4];
        case 4:
            encoded_byte4 = encoded_block[3];
        case 3:
            encoded_byte3 = encoded_block[2];
        case 2:
            encoded_byte2 = encoded_block[1];
        case 1:
            encoded_byte1 = encoded_block[0];
            decoded_bytes[decoded_base_idx]   = ((encoded_byte1 << 3) & 0xF8) | ((encoded_byte2 >> 2) & 0x07);
            decoded_bytes[decoded_base_idx+1] = ((encoded_byte2 << 6) & 0xC0) | ((encoded_byte3 << 1) & 0x3E) | ((encoded_byte4 >> 4) & 0x01);
            decoded_bytes[decoded_base_idx+2] = ((encoded_byte4 << 4) & 0xF0) | ((encoded_byte5 >> 1) & 0x0F);
            decoded_bytes[decoded_base_idx+3] = ((encoded_byte5 << 7) & 0x80) | ((encoded_byte6 << 2) & 0x7C) | ((encoded_byte7 >> 3) & 0x03);
            decoded_bytes[decoded_base_idx+4] = ((encoded_byte7 << 5) & 0xE0);
    }
    decoded_base_idx += padding_adjustment[encoded_block_idx];
    
    // Set decoded size
    *decoded_size = decoded_base_idx;
    return true;
}
