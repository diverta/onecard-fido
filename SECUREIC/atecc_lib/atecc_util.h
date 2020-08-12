/* 
 * File:   atecc_util.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:36
 */
#ifndef ATECC_UTIL_H
#define ATECC_UTIL_H

#include <stddef.h>
#include <stdint.h>

#include "atecc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ATECC_STATUS atecc_get_addr(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t* addr);
ATECC_STATUS atecc_get_zone_size(uint8_t zone, uint16_t slot, size_t* size);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_UTIL_H */
