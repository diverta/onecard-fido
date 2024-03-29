/* 
 * File:   platform_common.c
 * Author: makmorit
 *
 * Created on 2022/11/21, 14:25
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//
// LED操作関数群
//
#include "app_status_indicator.h"

void fido_status_indicator_none(void)
{
    app_status_indicator_none();
}

void fido_status_indicator_idle(void)
{
    app_status_indicator_idle();
}

void fido_status_indicator_busy(void)
{
    app_status_indicator_busy();
}

void fido_status_indicator_prompt_reset(void)
{
    app_status_indicator_prompt_reset();
}

void fido_status_indicator_prompt_tup(void)
{
    app_status_indicator_prompt_tup();
}

void fido_status_indicator_pairing_mode(void)
{
    app_status_indicator_pairing_mode();
}

void fido_status_indicator_pairing_fail(bool short_interval)
{
    if (short_interval) {
        app_status_indicator_connection_fail();
    } else {
        app_status_indicator_pairing_fail();
    }
}

void fido_status_indicator_abort(void)
{
    app_status_indicator_abort();
}

void fido_status_indicator_ble_scanning(void)
{
    app_status_indicator_ble_scanning();
}

//
// トランスポート関連
//
#include "app_ble_fido.h"
#include "app_ble_pairing.h"
#include "app_usb_ccid.h"
#include "app_usb_hid.h"

void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    app_usb_hid_send_report(buffer_for_send, size);
}

void usbd_ccid_send_data_frame(uint8_t *buffer_for_send, size_t size)
{
    app_usb_ccid_send_data(buffer_for_send, size);
}

bool fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy)
{
    return app_ble_fido_send_data(u2f_status_buffer, u2f_status_buffer_length);
}

bool fido_ble_pairing_mode_get(void)
{
    return app_ble_pairing_mode();
}

bool fido_ble_service_disconnected(void)
{
    // BLEクライアントとの接続が切り離されている場合 true
    return (app_ble_fido_connected() == false);
}

//
// 管理コマンド用関数群
//
#include "app_board.h"
#include "app_flash.h"

bool fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    return app_flash_get_stat_csv(stat_csv_data, stat_csv_size);
}

bool fido_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size)
{
    return app_board_get_version_info_csv(info_csv_data, info_csv_size);
}

void fido_board_system_reset(void)
{
    app_board_prepare_for_system_reset();
}

void ble_peripheral_auth_param_request(uint8_t *request, size_t request_size)
{
    (void)request;
    (void)request_size;
}

bool ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size)
{
    // nRF5340ではサポートしません。
    (void)cmd_type;
    (void)response;
    (void)response_size;
    return false;
}

//
// [Dummy] DFU関連
//
bool usbd_service_support_bootloader_mode(void)
{
    return false;
}

void usbd_service_stop_for_bootloader(void)
{
}

//
// [Dummy] BLE自動認証関連
//
void ble_peripheral_auth_param_init(void)
{
}

bool ble_peripheral_auth_scan_enable(void)
{
    return false;
}

bool ble_peripheral_auth_start_scan(void *context)
{
    (void)context;
    return false;
}

size_t ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff)
{
    (void)p_buff;
    return 0;
}
