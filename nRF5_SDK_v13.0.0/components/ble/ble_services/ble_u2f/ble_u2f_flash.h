#ifndef BLE_U2F_FLASH_H__
#define BLE_U2F_FLASH_H__

#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ble_u2f_flash_keydata_delete(void);
bool ble_u2f_flash_keydata_read(ble_u2f_context_t *p_u2f_context);
bool ble_u2f_flash_keydata_write(ble_u2f_context_t *p_u2f_context);
bool ble_u2f_flash_token_counter_write(ble_u2f_context_t *p_u2f_context, uint8_t *p_appid_hash, uint32_t token_counter, uint32_t reserve_word);
bool ble_u2f_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t ble_u2f_flash_token_counter_value(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_FLASH_H__

/** @} */
