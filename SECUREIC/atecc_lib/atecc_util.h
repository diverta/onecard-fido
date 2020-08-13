/* 
 * File:   atecc_util.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:36
 */
#ifndef ATECC_UTIL_H
#define ATECC_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atecc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ATECC_STATUS atecc_get_address(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t *addr);
ATECC_STATUS atecc_get_zone_size(uint8_t zone, uint16_t slot, size_t *size);
ATECC_STATUS atecc_lock_config_zone(void);
ATECC_STATUS atecc_lock_data_zone(void);
bool         atecc_lock_status_get(uint8_t zone, bool *is_locked);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_UTIL_H */
