/* 
 * File:   ccid_flash_object.h
 * Author: makmorit
 *
 * Created on 2021/02/15, 11:58
 */
#ifndef CCID_FLASH_OBJECT_H
#define CCID_FLASH_OBJECT_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *ccid_flash_object_read_buffer(void);
uint8_t    *ccid_flash_object_write_buffer(void);
size_t      ccid_flash_object_rw_buffer_size(void);
bool        ccid_flash_object_read_by_tag(CCID_APPLET applet_id, uint8_t obj_tag, bool *is_exist, uint8_t *buff, size_t *size);
bool        ccid_flash_object_write_by_tag(CCID_APPLET applet_id, uint8_t obj_tag, uint8_t *obj_buff, size_t obj_size);

//
// コールバック関数群
//
void        ccid_flash_object_failed(void);
void        ccid_flash_object_gc_done(void);
void        ccid_flash_object_record_updated(void);
void        ccid_flash_object_record_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_FLASH_OBJECT_H */
