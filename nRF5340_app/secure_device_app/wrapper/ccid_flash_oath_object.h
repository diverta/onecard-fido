/* 
 * File:   ccid_flash_oath_object.h
 * Author: makmorit
 *
 * Created on 2022/06/15, 15:09
 */
#ifndef CCID_FLASH_OATH_OBJECT_H
#define CCID_FLASH_OATH_OBJECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        ccid_flash_oath_object_write(uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size, bool use_serial, uint16_t serial);
bool        ccid_flash_oath_object_find(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial);
bool        ccid_flash_oath_object_delete(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial);

//
// コールバック関数群
//
void        ccid_flash_oath_object_record_updated(void);
void        ccid_flash_oath_object_record_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_FLASH_OATH_OBJECT_H */
