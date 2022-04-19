/* 
 * File:   ccid_flash_piv_object.h
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#ifndef CCID_FLASH_PIV_OBJECT_H
#define CCID_FLASH_PIV_OBJECT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg, bool *is_exist);
bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg);

bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t *key_size, bool *is_exist);
bool ccid_flash_piv_object_private_key_write(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t key_size);

bool ccid_flash_piv_object_pin_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size);

bool ccid_flash_piv_object_data_read(uint8_t obj_tag, uint8_t *obj_data, size_t *obj_size, bool *is_exist);
bool ccid_flash_piv_object_data_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size);
bool ccid_flash_piv_object_data_erase(void);

void ccid_flash_piv_object_failed(void);
void ccid_flash_piv_object_gc_done(void);
void ccid_flash_piv_object_record_updated(void);
void ccid_flash_piv_object_record_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_FLASH_PIV_OBJECT_H */
