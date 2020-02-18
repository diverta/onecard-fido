/* 
 * File:   fido_flash_token_counter.h
 * Author: makmorit
 *
 * Created on 2019/07/09, 10:22
 */
#ifndef FIDO_FLASH_TOKEN_COUNTER_H
#define FIDO_FLASH_TOKEN_COUNTER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool     fido_flash_token_counter_delete(void);
bool     fido_flash_token_counter_write(uint8_t *p_unique_key, uint32_t token_counter, uint8_t *p_rpid_hash, uint8_t *p_username, size_t username_size, uint16_t key_id);
bool     fido_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t fido_flash_token_counter_value(void);
uint8_t *fido_flash_token_counter_get_check_hash(void);
uint8_t *fido_flash_token_counter_user_name(void);
uint16_t fido_flash_token_counter_key_id(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_TOKEN_COUNTER_H */
