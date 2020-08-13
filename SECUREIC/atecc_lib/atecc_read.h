/* 
 * File:   atecc_read.h
 * Author: makmorit
 *
 * Created on 2020/08/11, 11:26
 */
#ifndef ATECC_READ_H
#define ATECC_READ_H

#include "atecc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

ATECC_STATUS atecc_read_bytes_zone(uint8_t zone, uint16_t slot, size_t offset, uint8_t *data, size_t length);
ATECC_STATUS atecc_read_config_zone(uint8_t* config_data);
ATECC_STATUS atecc_read_serial_number(uint8_t* serial_number);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_READ_H */
