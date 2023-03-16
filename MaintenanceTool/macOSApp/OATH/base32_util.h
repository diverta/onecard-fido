//
//  base32_util.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/16.
//
#ifndef base32_util_h
#define base32_util_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool base32_decode(uint8_t *decoded, size_t *decoded_size, const char *encoded);

#endif /* base32_util_h */
