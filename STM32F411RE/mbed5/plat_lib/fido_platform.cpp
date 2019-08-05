/* 
 * File:   fido_platform.cpp
 * Author: makmorit
 *
 * Created on 2019/07/25, 16:00
 */
#include "mbed.h"

#include "fido_board.h"
#include "fido_flash.h"
#include "fido_log.h"
#include "fido_status_indicator.h"
#include "usbd_hid_service.h"

#include "fido_platform.h"
#include "fido_hid_channel.h"

//
// main.cppとのインターフェース
//
void application_initialize(void)
{
    // FIDO Authenticator固有のタイマー機能
    fido_button_timers_init();

    //
    // USB HIDデバイスを初期化
    //  PC-->mbed: 64バイト
    //  mbed-->PC: 64バイト
    //
    usbd_hid_init();
    
    // アプリケーションで使用するFlash ROM機能の初期化
    fido_flash_init();

    // アプリケーションで使用するボタンの設定
    fido_button_init();

    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();

    // TODO:PINトークンとキーペアを再生成
    // ctap2_client_pin_init();
    
    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();
}

bool application_main(void)
{
    usbd_hid_do_process();

    wait(0.01);
    return true;
}

//
// fido_ble_pairing.c
//
bool     fido_ble_pairing_mode_get(void) 
{
    return true;
}

//
// fido_ble_service.c
//
bool     fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy)
{
    return true;
}
bool     fido_ble_service_disconnected(void)
{
    return true;
}
void     fido_ble_service_disconnect_force(void)
{
}

//
// fido_ble_send_retry.c
//
void fido_ble_send_retry_timer_start(void)
{
}


//
// fido_crypto.c
//
void fido_crypto_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{
}
void fido_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
}
void fido_crypto_ecdsa_sign(uint8_t *private_key_be, uint8_t const *hash_digest, size_t digest_size, uint8_t *signature, size_t *signature_size)
{
}
void fido_crypto_calculate_hmac_sha256(uint8_t *key_data, size_t key_data_size, uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size, uint8_t *dest_data)
{
}

//
// fido_crypto_aes_cbc_256.c
//
size_t fido_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    return 0;
}
size_t fido_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    return 0;
}

//
// fido_crypto_keypair.c
//
void     fido_crypto_keypair_generate(void)
{
}
uint8_t *fido_crypto_keypair_private_key(void)
{
    return 0;
}
uint8_t *fido_crypto_keypair_public_key(void)
{
    return 0;
}
size_t   fido_crypto_keypair_private_key_size(void)
{
    return 0;
}

//
// fido_crypto_sskey.c
//
void     fido_crypto_sskey_init(bool force)
{
}
uint8_t  fido_crypto_sskey_generate(uint8_t *client_public_key_raw_data)
{
    return 0;
}
uint8_t *fido_crypto_sskey_public_key(void)
{
    return 0;
}
uint8_t *fido_crypto_sskey_hash(void)
{
    return 0;
}

//
// fido_flash.c
//
bool      fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    return true;
}

//
// fido_flash_skey_cert.c
//
bool      fido_flash_skey_cert_delete(void)
{
    return true;
}
bool      fido_flash_skey_cert_write(void)
{
    return true;
}
bool      fido_flash_skey_cert_read(void)
{
    return true;
}
bool      fido_flash_skey_cert_available(void)
{
    return true;
}
bool      fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length)
{
    return true;
}
uint32_t *fido_flash_skey_cert_data(void)
{
    return 0;
}
uint8_t  *fido_flash_skey_data(void)
{
    return 0;
}
uint8_t  *fido_flash_cert_data(void)
{
    return 0;
}
uint32_t  fido_flash_cert_data_length(void)
{
    return 0;
}

//
// fido_flash_token_counter.c
//
bool      fido_flash_token_counter_delete(void)
{
    return true;
}
bool      fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint8_t *p_hash_for_check)
{
    return true;
}
bool      fido_flash_token_counter_read(uint8_t *p_appid_hash)
{
    return true;
}
uint32_t  fido_flash_token_counter_value(void)
{
    return 0;
}
uint8_t  *fido_flash_token_counter_get_check_hash(void)
{
    return 0;
}

//
// fido_flash_client_pin_store.c
//
bool      fido_flash_client_pin_store_hash_read(void)
{
    return true;
}
bool      fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter)
{
    return true;
}
uint8_t  *fido_flash_client_pin_store_pin_code_hash(void)
{
    return 0;
}
uint32_t  fido_flash_client_pin_store_retry_counter(void)
{
    return 0;
}
bool      fido_flash_client_pin_store_pin_code_exist(void)
{
    return true;
}

//
// fido_flash_password.c
//
uint8_t *fido_flash_password_get(void)
{
    return 0;
}
bool     fido_flash_password_set(uint8_t *random_vector)
{
    return true;
}

//
// fido_log.c
//
void fido_log_print_hexdump_debug(uint8_t *data, size_t size)
{
    _fido_log_print_hexdump_debug(data, size);
}

//
// fido_status_indicator.c
//
void fido_status_indicator_none(void)
{
    _fido_status_indicator_none();
}
void fido_status_indicator_idle(void)
{
    _fido_status_indicator_idle();
}
void fido_status_indicator_prompt_reset(void)
{
    _fido_status_indicator_prompt_reset();
}
void fido_status_indicator_prompt_tup(void)
{
    _fido_status_indicator_prompt_tup();
}
void fido_status_indicator_pairing_mode(void)
{
    _fido_status_indicator_pairing_mode();
}
void fido_status_indicator_pairing_fail(void)
{
    _fido_status_indicator_pairing_fail();
}
void fido_status_indicator_abort(void)
{
    _fido_status_indicator_abort();
}

//
// fido_timer.c
//
void fido_user_presence_verify_timer_stop(void)
{
}
void fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context)
{
}
void fido_keepalive_interval_timer_stop(void)
{
}
void fido_keepalive_interval_timer_start(uint32_t timeout_msec, void *p_context)
{
}
void fido_hid_channel_lock_timer_stop(void)
{
}
void fido_hid_channel_lock_timer_start(uint32_t lock_ms)
{
}

//
// nfc_service.c
//
void nfc_service_data_send(uint8_t *data, size_t data_size)
{
}

//
// usbd_hid_service.c
//
void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    _usbd_hid_frame_send(buffer_for_send, size);
}
