/* 
 * File:   ccid_flash_openpgp_object.h
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#ifndef CCID_FLASH_OPENPGP_OBJECT_H
#define CCID_FLASH_OPENPGP_OBJECT_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        ccid_flash_openpgp_object_read(CCID_APPLET applet_id, uint16_t obj_tag, bool *is_exist, uint8_t *buff, size_t *size);
bool        ccid_flash_openpgp_object_write(CCID_APPLET applet_id, uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size);
bool        ccid_flash_openpgp_object_delete_all(CCID_APPLET applet_id);

//
// コールバック関数群
//
void        ccid_flash_openpgp_object_failed(void);
void        ccid_flash_openpgp_object_gc_done(void);
void        ccid_flash_openpgp_object_record_updated(void);
void        ccid_flash_openpgp_object_record_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_FLASH_OPENPGP_OBJECT_H */
