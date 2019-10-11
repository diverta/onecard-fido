/* 
 * File:   ble_service_central_demo.c
 * Author: makmorit
 *
 * Created on 2019/10/10, 16:55
 */
// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_central_demo
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// デモ機能
//
#include "ble_service_central.h"
#include "ble_service_central_stat.h"

static void resume_function_after_scan(void)
{
    // 統計情報をデバッグ出力
    ble_service_central_stat_debug_print();

    //
    // One cardダミーデバイスが見つかったらプリントして終了
    // （422E0000-E141-11E5-A837-0800200C9A66）
    //
    char *one_card_uuid_string = "422E0000-E141-11E5-A837-0800200C9A66";
    ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(one_card_uuid_string);
    if (info == NULL) {
        NRF_LOG_DEBUG("One card peripheral device not found.");
    } else {
        NRF_LOG_DEBUG("One card peripheral device found: NAME(%s) ADDR(%s)", 
            info->dev_name, ble_service_central_stat_btaddr_string(info->peer_addr));
    }
}

void ble_service_central_demo_button_pressed(void)
{
    // ボタン押下で、BLEダミーデバイスをスキャンし、
    // 見つかった場合、ログをプリント
    ble_service_central_scan_start(1000, resume_function_after_scan);
}
