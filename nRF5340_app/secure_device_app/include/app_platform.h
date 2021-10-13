/* 
 * File:   app_platform.h
 * Author: makmorit
 *
 * Created on 2021/08/19, 9:52
 */
#ifndef APP_PLATFORM_H
#define APP_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>
#include <zephyr.h>

//
// ログ出力関連
//
#include <logging/log.h>

#define fido_log_module_register(n)         LOG_MODULE_REGISTER(n)
#define fido_log_info(...)                  LOG_INF(__VA_ARGS__)
#define fido_log_warning(...)               LOG_WRN(__VA_ARGS__)
#define fido_log_error(...)                 LOG_ERR(__VA_ARGS__)
#define fido_log_debug(...)                 LOG_DBG(__VA_ARGS__)
#define fido_log_print_hexdump_debug(...)   LOG_HEXDUMP_DBG(__VA_ARGS__)

//
// LED操作関数群
//
#include "app_status_indicator.h"

#define fido_status_indicator_none          app_status_indicator_none
#define fido_status_indicator_idle          app_status_indicator_idle
#define fido_status_indicator_busy          app_status_indicator_busy
#define fido_status_indicator_prompt_reset  app_status_indicator_prompt_reset
#define fido_status_indicator_prompt_tup    app_status_indicator_prompt_tup
#define fido_status_indicator_pairing_mode  app_status_indicator_pairing_mode
#define fido_status_indicator_pairing_fail  app_status_indicator_pairing_fail
#define fido_status_indicator_abort         app_status_indicator_abort
#define fido_status_indicator_ble_scanning  app_status_indicator_ble_scanning

//
// 管理コマンド用関数群
//
bool        fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size);
bool        fido_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size);
void        ble_peripheral_auth_param_request(uint8_t *request, size_t request_size);
bool        ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool        ble_service_common_erase_bond_data(void (*_response_func)(bool));

//
// [Dummy] BLE自動認証関連
//
void        ble_peripheral_auth_param_init(void);
bool        ble_peripheral_auth_scan_enable(void);
bool        ble_peripheral_auth_start_scan(void *context);
size_t      ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff);

//
// FIDOトランスポート用関数群
//
bool        fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy);
bool        fido_ble_pairing_mode_get(void);
bool        fido_ble_service_disconnected(void);

//
// タイマー関連
//
void        fido_user_presence_verify_timer_stop(void);
void        fido_user_presence_verify_timer_start(uint32_t timeout_msec, void *p_context);
void        fido_hid_channel_lock_timer_stop(void);
void        fido_hid_channel_lock_timer_start(uint32_t lock_ms);
void        fido_repeat_process_timer_stop(void);
void        fido_repeat_process_timer_start(uint32_t timeout_msec, void (*handler)(void));

//
// 永続化関連
//
bool        fido_flash_token_counter_delete(void);
bool        fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint8_t *p_hash_for_check);
bool        fido_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t    fido_flash_token_counter_value(void);
uint8_t    *fido_flash_token_counter_get_check_hash(void);

//
// 関数群
//
void        usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);
bool        usbd_service_support_bootloader_mode(void);
void        usbd_service_stop_for_bootloader(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_PLATFORM_H */
