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

// LED点灯モード
typedef enum _LED_LIGHT_MODE {
    LED_LIGHT_NONE = 0,
    LED_LIGHT_FOR_PAIRING_MODE,
    LED_LIGHT_FOR_USER_PRESENCE,
    LED_LIGHT_FOR_PROCESSING
} LED_LIGHT_MODE;

// LED点滅間隔の定義
#define LED_ON_OFF_INTERVAL_MSEC        300
#define LED_ON_OFF_SHORT_INTERVAL_MSEC  100
#define LED_BLINK_INTERVAL_MSEC         250

// キープアライブ・タイマー
#define U2F_KEEPALIVE_INTERVAL_MSEC   500
#define CTAP2_KEEPALIVE_INTERVAL_MSEC 200

//
// fido_board.c
//
void fido_led_light(LED_LIGHT_MODE led_light_mode, bool led_on);
void fido_led_light_all(bool led_on);
void fido_processing_led_on(LED_LIGHT_MODE led_light_mode, uint32_t on_off_interval_msec);
void fido_processing_led_off(void);
void fido_idling_led_on(void);
void fido_idling_led_off(void);

//
// fido_command.c
//
void    fido_user_presence_terminate(void);
void    fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context);
uint8_t fido_user_presence_verify_end(void);

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
// fido_flash_password.c
//
bool     fido_flash_password_generate(void);
uint8_t *fido_flash_password_get(void);

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
