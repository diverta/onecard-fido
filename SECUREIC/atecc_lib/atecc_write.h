/* 
 * File:   atecc_write.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:27
 */
#ifndef ATECC_WRITE_H
#define ATECC_WRITE_H

#include <stdint.h>

#include "atecc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ATECC_STATUS atecc_write_config_zone(const uint8_t *config_data);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_WRITE_H */
