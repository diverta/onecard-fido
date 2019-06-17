/* 
 * File:   fido_flash_client_pin_store.h
 * Author: makmorit
 *
 * Created on 2019/02/27, 10:43
 */
#ifndef FIDO_FLASH_CLIENT_PIN_STORE_H
#define FIDO_FLASH_CLIENT_PIN_STORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool      fido_flash_client_pin_store_hash_read(void);
bool      fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);
uint8_t  *fido_flash_client_pin_store_pin_code_hash(void);
uint32_t  fido_flash_client_pin_store_retry_counter(void);
bool      fido_flash_client_pin_store_pin_code_exist(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_CLIENT_PIN_STORE_H */
