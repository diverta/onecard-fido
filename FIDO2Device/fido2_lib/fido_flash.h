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

bool        fido_flash_client_pin_store_hash_read(void);
uint8_t    *fido_flash_client_pin_store_pin_code_hash(void);
uint32_t    fido_flash_client_pin_store_retry_counter(void);
bool        fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_H */
