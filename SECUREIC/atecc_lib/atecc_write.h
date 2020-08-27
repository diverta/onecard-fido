/* 
 * File:   atecc_write.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:27
 */
#ifndef ATECC_WRITE_H
#define ATECC_WRITE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool atecc_write_zone(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, const uint8_t *data, uint8_t len);
bool atecc_write_bytes_zone(uint8_t zone, uint16_t slot, size_t offset_bytes, const uint8_t *data, size_t length);
bool atecc_write_config_zone(const uint8_t *config_data);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_WRITE_H */
