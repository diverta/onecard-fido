/* 
 * File:   app_platform.c
 * Author: makmorit
 *
 * Created on 2021/08/19, 9:52
 */
#include <zephyr/types.h>
#include <zephyr.h>

//
// トランスポート関連
//
#include "app_ble_pairing.h"
#include "app_usb_hid.h"

void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    app_usb_hid_send_report(buffer_for_send, size);
}

bool fido_ble_response_send(uint8_t *u2f_status_buffer, size_t u2f_status_buffer_length, bool *busy)
{
    return app_ble_fido_send_data(u2f_status_buffer, u2f_status_buffer_length);
}

bool fido_ble_pairing_mode_get(void)
{
    return app_ble_pairing_mode();
}

//
// DFU関連
//
#include "app_dfu.h"

void usbd_service_stop_for_bootloader(void)
{
    // ブートローダーモード遷移を指示
    app_dfu_prepare_for_bootloader();
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

void ble_peripheral_auth_param_request(uint8_t *request, size_t request_size)
{
    (void)request;
    (void)request_size;
}

bool ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size)
{
    (void)cmd_type;
    (void)response;
    (void)response_size;
    return false;
}

bool ble_service_common_erase_bond_data(void (*_response_func)(bool))
{
    // TODO:
    // Zephyrでは、現状のCONFIGだと
    // ペアリング情報が永続化されない様子。
    // なので当面、別段の処理を実行せず、
    // 常に正常レスポンスを戻すようにする
    
    // ペアリング情報削除後に実行される処理
    if (_response_func != NULL) {
        (*_response_func)(true);
    }
    return true;
}
