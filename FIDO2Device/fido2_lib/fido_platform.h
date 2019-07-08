/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// LED点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

// キープアライブ・タイマー
#define U2F_KEEPALIVE_INTERVAL_MSEC   500
#define CTAP2_KEEPALIVE_INTERVAL_MSEC 200

// crypto関連の定義
#define RAW_PRIVATE_KEY_SIZE    32
#define RAW_PUBLIC_KEY_SIZE     64
#define SHARED_SECRET_SIZE      32
#define ECDSA_SIGNATURE_SIZE    64
#define SHA_256_HASH_SIZE       32
#define SSKEY_HASH_SIZE         32
#define HMAC_SHA_256_SIZE       32

// fido_ble_pairing.c
bool     fido_ble_pairing_mode_get(void);

//
// fido_ble_service.c
//
uint32_t fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length);
bool     fido_ble_service_disconnected(void);
void     fido_ble_service_disconnect_force(void);

//
// fido_ble_send_retry.c
//
void fido_ble_send_retry_timer_start(void);

//
// fido_board.c
//
void fido_led_light_all(bool led_on);
void fido_prompt_led_blink_start(uint32_t on_off_interval_msec);
void fido_caution_led_blink_start(uint32_t on_off_interval_msec);
void fido_led_blink_stop(void);
void fido_idling_led_blink_start(void);
void fido_idling_led_blink_stop(void);

//
// fido_command.c
//
bool    fido_command_do_abort(void);
void    fido_command_abort_flag_set(bool flag);
void    fido_user_presence_terminate(void);
void    fido_user_presence_verify_start(uint32_t timeout_msec);
uint8_t fido_user_presence_verify_end(void);

//
// fido_crypto.c
//
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size);
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size);
void fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size);
void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data);

//
// fido_crypto_aes_cbc_256.c
//
size_t fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted);
size_t fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted);

//
// fido_crypto_keypair.c
//
void     fido_crypto_keypair_generate(void);
uint8_t *fido_crypto_keypair_private_key(void);
uint8_t *fido_crypto_keypair_public_key(void);
size_t   fido_crypto_keypair_private_key_size(void);

//
// fido_crypto_sskey.c
//
void     fido_crypto_sskey_init(bool force);
uint8_t  fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *fido_crypto_sskey_public_key(void);
uint8_t *fido_crypto_sskey_hash(void);

//
// fido_flash.c
//
bool      fido_flash_skey_cert_delete(void);
bool      fido_flash_skey_cert_write(void);
bool      fido_flash_skey_cert_read(void);
bool      fido_flash_skey_cert_available(void);
bool      fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
uint32_t *fido_flash_skey_cert_data(void);
uint8_t  *fido_flash_skey_data(void);
uint8_t  *fido_flash_cert_data(void);
uint32_t  fido_flash_cert_data_length(void);

bool      fido_flash_token_counter_delete(void);
bool      fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint8_t *p_hash_for_check);
bool      fido_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t  fido_flash_token_counter_value(void);
uint8_t  *fido_flash_token_counter_get_check_hash(void);

//
// fido_flash_client_pin_store.c
//
bool      fido_flash_client_pin_store_hash_read(void);
bool      fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);
uint8_t  *fido_flash_client_pin_store_pin_code_hash(void);
uint32_t  fido_flash_client_pin_store_retry_counter(void);
bool      fido_flash_client_pin_store_pin_code_exist(void);

//
// fido_flash_event_t
//   FDSイベント(Nordic)の読替用構造体
//
typedef struct {
    // FDSイベントを判定して設定する項目
    bool result;
    bool gc;
    bool delete_file;
    bool write_update;
    bool retry_counter_write;
    bool token_counter_write;
    bool skey_cert_write;
    bool aeskeys_write;
    bool pairing_mode_write;
    // アプリケーションで任意に設定する項目
    bool gc_forced;
} fido_flash_event_t;

//
// fido_flash_password.c
//
uint8_t *fido_flash_password_get(void);
bool     fido_flash_password_set(uint8_t *random_vector);

//
// fido_log.h
//
// nRF52840の場合は、NRF_LOG_xxxx読替マクロが実体のため、
// nRF5 SDK依存のマクロ定義をインクルードさせる
#ifdef NRF52840_XXAA
#include "fido_log.h"
#endif

//
// fido_timer.c
//
void fido_comm_interval_timer_stop(void);
void fido_comm_interval_timer_start(void);
void fido_processing_led_timer_stop(void);
void fido_processing_led_timer_start(uint32_t on_off_interval_msec);
void fido_lock_channel_timer_stop(void);
void fido_lock_channel_timer_start(uint32_t lock_ms);

//
// nfc_service.c
//
void nfc_service_data_send(uint8_t *data, size_t data_size);

//
// usbd_hid_service.c
//
void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
