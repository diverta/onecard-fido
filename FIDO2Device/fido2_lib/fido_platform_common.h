/* 
 * File:   fido_platform_common.h
 * Author: makmorit
 *
 * Created on 2020/02/11, 9:31
 */
#ifndef FIDO_PLATFORM_COMMON_H
#define FIDO_PLATFORM_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// crypto関連の定義
#define RAW_PRIVATE_KEY_SIZE    32
#define RAW_PUBLIC_KEY_SIZE     64
#define SHARED_SECRET_SIZE      32
#define ECDSA_SIGNATURE_SIZE    64
#define SHA_256_HASH_SIZE       32
#define SSKEY_HASH_SIZE         32
#define HMAC_SHA_256_SIZE       32

// BLEデバイスによる自動認証機能
#include "demo_ble_peripheral_auth.h"

// fido_ble_pairing.c
bool fido_ble_pairing_mode_get(void);

//
// fido_ble_service.c
//
bool fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy);
bool fido_ble_service_disconnected(void);
void fido_ble_service_disconnect_force(void);

//
// fido_board.c
//
bool fido_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size);

//
// fido_crypto_aes_cbc_256.c
//
size_t fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);

//
// fido_flash_common.c
//
bool fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size);

//
// fido_flash_client_pin_store.c
//
bool     fido_flash_client_pin_store_hash_read(void);
bool     fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);
uint8_t *fido_flash_client_pin_store_pin_code_hash(void);
uint32_t fido_flash_client_pin_store_retry_counter(void);
bool     fido_flash_client_pin_store_pin_code_exist(void);

//
// fido_flash_blp_auth_param.c
//
bool     fido_flash_blp_auth_param_read(void);
bool     fido_flash_blp_auth_param_write(uint8_t *p_uuid_string, uint32_t scan_sec, uint32_t scan_enable);
uint8_t *fido_flash_blp_auth_param_service_uuid_string(void);
uint32_t fido_flash_blp_auth_param_service_uuid_scan_sec(void);
uint32_t fido_flash_blp_auth_param_service_uuid_scan_enable(void);

//
// fido_log.h
//
//  プラットフォームに固有なヘッダーファイル
//  "fido_log.h"を用意し、それをインクルード
// 
#include "fido_log.h"

//
// fido_status_indicator.c
//
void fido_status_indicator_none(void);
void fido_status_indicator_idle(void);
void fido_status_indicator_busy(void);
void fido_status_indicator_prompt_reset(void);
void fido_status_indicator_prompt_tup(void);
void fido_status_indicator_pairing_mode(void);
void fido_status_indicator_pairing_fail(void);
void fido_status_indicator_abort(void);
void fido_status_indicator_ble_scanning(void);

//
// fido_timer.c
//
void fido_user_presence_verify_timer_stop(void);
void fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context);
void fido_hid_channel_lock_timer_stop(void);
void fido_hid_channel_lock_timer_start(uint32_t lock_ms);
void fido_repeat_process_timer_stop(void);
void fido_repeat_process_timer_start(uint32_t timeout_msec, void (*handler)(void));

//
// nfc_service.c
//
void nfc_service_data_send(uint8_t *data, size_t data_size);

//
// usbd_hid_service.c
//
void usbd_service_stop_for_bootloader(void);

//
// CCID関連
//
#include "usbd_service_ccid.h"
#include "usbd_service_hid.h"
#include "ccid_flash_piv_object.h"

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_COMMON_H */
