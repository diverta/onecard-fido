/* 
 * File:   platform_common.h
 * Author: makmorit
 *
 * Created on 2022/11/21, 14:25
 */
#ifndef PLATFORM_COMMON_H
#define PLATFORM_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// LED操作関数群
//
void        fido_status_indicator_none(void);
void        fido_status_indicator_idle(void);
void        fido_status_indicator_busy(void);
void        fido_status_indicator_prompt_reset(void);
void        fido_status_indicator_prompt_tup(void);
void        fido_status_indicator_pairing_mode(void);
void        fido_status_indicator_pairing_fail(bool short_interval);
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
void        fido_board_system_reset(void);
void        ble_peripheral_auth_param_request(uint8_t *request, size_t request_size);
bool        ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size);
bool        ble_service_common_erase_bond_data(void (*_response_func)(bool));

//
// [Dummy] DFU関連
//
bool        usbd_service_support_bootloader_mode(void);
void        usbd_service_stop_for_bootloader(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_COMMON_H */
