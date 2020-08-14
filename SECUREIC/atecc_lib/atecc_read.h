/* 
 * File:   atecc_read.h
 * Author: makmorit
 *
 * Created on 2020/08/11, 11:26
 */
#ifndef ATECC_READ_H
#define ATECC_READ_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool atecc_read_bytes_zone(uint8_t zone, uint16_t slot, size_t offset, uint8_t *data, size_t length);
bool atecc_read_config_zone(uint8_t *config_data);
bool atecc_read_serial_number(uint8_t *serial_number);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_READ_H */
