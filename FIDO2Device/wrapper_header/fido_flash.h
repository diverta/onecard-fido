/* 
 * File:   fido_flash.h
 * Author: makmorit
 *
 * Created on 2021/09/20, 11:14
 */
#ifndef FIDO_FLASH_H
#define FIDO_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint32_t   *fido_flash_skey_cert_data(void);
uint8_t    *fido_flash_skey_data(void);
uint8_t    *fido_flash_cert_data(void);
uint32_t    fido_flash_cert_data_length(void);
bool        fido_flash_skey_cert_read(void);
bool        fido_flash_skey_cert_available(void);
bool        fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
bool        fido_flash_skey_cert_write(void);
bool        fido_flash_skey_cert_delete(void);

uint8_t    *fido_flash_password_get(void);
bool        fido_flash_password_set(uint8_t *random_vector);

bool        fido_flash_token_counter_delete(void);
uint32_t    fido_flash_token_counter_value(void);
uint8_t    *fido_flash_token_counter_get_check_hash(void);
bool        fido_flash_token_counter_read(uint8_t *p_unique_key);
bool        fido_flash_token_counter_write(uint8_t *p_unique_key, uint32_t token_counter, uint8_t *p_rpid_hash);

bool        fido_flash_client_pin_store_hash_read(void);
uint8_t    *fido_flash_client_pin_store_pin_code_hash(void);
uint32_t    fido_flash_client_pin_store_retry_counter(void);
bool        fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);
bool        fido_flash_client_pin_store_pin_code_exist(void);

//
// コールバック関数群
//
void        fido_flash_object_failed(void);
void        fido_flash_object_gc_done(void);
void        fido_flash_object_record_updated(void);
void        fido_flash_object_record_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_H */
