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
void        fido_status_indicator_none(void);
void        fido_status_indicator_idle(void);
void        fido_status_indicator_busy(void);
void        fido_status_indicator_prompt_reset(void);
void        fido_status_indicator_prompt_tup(void);
void        fido_status_indicator_pairing_mode(void);
void        fido_status_indicator_pairing_fail(void);
void        fido_status_indicator_abort(void);
void        fido_status_indicator_ble_scanning(void);

//
// FIDOトランスポート用関数群
//
void        usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size);
void        usbd_ccid_send_data_frame(uint8_t *buffer_for_send, size_t size);
bool        fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy);
bool        fido_ble_pairing_mode_get(void);
bool        fido_ble_service_disconnected(void);

//
// 管理コマンド用関数群
//
bool        fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size);
bool        fido_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size);
void        ble_peripheral_auth_param_request(uint8_t *request, size_t request_size);
bool        ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool        ble_service_common_erase_bond_data(void (*_response_func)(bool));

//
// [Dummy] DFU関連
//
bool        usbd_service_support_bootloader_mode(void);
void        usbd_service_stop_for_bootloader(void);

//
// [Dummy] BLE自動認証関連
//
void        ble_peripheral_auth_param_init(void);
bool        ble_peripheral_auth_scan_enable(void);
bool        ble_peripheral_auth_start_scan(void *context);
size_t      ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff);

#ifdef __cplusplus
}
#endif

#endif /* APP_PLATFORM_H */
