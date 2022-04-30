/* 
 * File:   ccid_flash_openpgp_object.h
 * Author: makmorit
 *
 * Created on 2021/03/01, 9:52
 */
#ifndef CCID_FLASH_OPENPGP_OBJECT_H
#define CCID_FLASH_OPENPGP_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        ccid_flash_openpgp_object_read(uint8_t applet_id, uint16_t obj_tag, bool *is_exist, uint8_t *buff, size_t *size);
bool        ccid_flash_openpgp_object_write(uint8_t applet_id, uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size);
bool        ccid_flash_openpgp_object_delete_all(uint8_t applet_id);

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
