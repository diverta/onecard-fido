#ifndef BLE_U2F_CRYPTO_ECB_H__
#define BLE_U2F_CRYPTO_ECB_H__

#include <stdint.h>

// for nrf_value_length_t
#include "nrf_crypto_types.h"

#ifdef __cplusplus
extern "C" {
#endif


bool ble_u2f_crypto_ecb_init(void);
void ble_u2f_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);
void ble_u2f_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);

void ble_u2f_crypto_ecb_generate_keyhandle(uint8_t *p_appid_hash, nrf_value_length_t *private_key);
void ble_u2f_crypto_ecb_restore_keyhandle_base(nrf_value_length_t *keyhandle);

// キーハンドル生成・格納用領域
// Register, Authenticateで共通使用
extern uint8_t keyhandle_base_buffer[64];
extern uint8_t keyhandle_buffer[64];


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_CRYPTO_ECB_H__

/** @} */
