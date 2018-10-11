/* 
 * File:   one_card_event.c
 * Author: makmorit
 *
 * Created on 2018/10/09, 11:59
 */

#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"

// for logging informations
#define NRF_LOG_MODULE_NAME one_card_event
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for FIDO
#include "ble_u2f.h"
#include "one_card_main.h"
#include "ble_u2f_command.h"
#include "ble_u2f_comm_interval_timer.h"
#include "ble_u2f_util.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_user_presence.h"

static void ble_u2f_on_connect(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    // U2Fクライアントとの接続が行われた時は、
    // 接続ハンドルを保持する
    p_u2f->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    // 共有情報を初期化する
    ble_u2f_command_initialize_context();

    // ユーザー所在確認のためのキープアライブタイマーを生成する
    ble_u2f_user_presence_init();

    // 無通信タイマーが既にスタートしている場合は停止させる
    ble_u2f_comm_interval_timer_stop(p_u2f);

    // FIDO機能実行中LEDを点灯
    ble_u2f_led_light_LED(p_u2f->led_for_processing_fido, true);
}

static void ble_u2f_on_disconnect(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    // U2Fクライアントとの接続が切り離された時は、
    // 接続ハンドルをクリアする
    UNUSED_PARAMETER(p_ble_evt);
    p_u2f->conn_handle = BLE_CONN_HANDLE_INVALID;

    // 共有情報を消去する
    ble_u2f_command_finalize_context();

    // FIDO機能実行中LEDを消灯
    ble_u2f_led_light_LED(p_u2f->led_for_processing_fido, false);
    
    // ペアリングモードをキャンセルするため、ソフトデバイスを再起動
    ble_u2f_pairing_on_disconnect();
}

bool one_card_ble_evt_handler(ble_evt_t *p_ble_evt, void *p_context)
{
    if (p_ble_evt == NULL) {
        return false;
    }
    NRF_LOG_DEBUG("BLE event id=0x%02x", p_ble_evt->header.evt_id);
    
    ble_u2f_t *p_u2f = one_card_get_U2F_context();
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            ble_u2f_on_connect(p_u2f, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            ble_u2f_on_disconnect(p_u2f, p_ble_evt);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            // ペアリングが成功したかどうかを判定
            ble_u2f_pairing_on_evt_auth_status(p_u2f, p_ble_evt);
            break;

        default:
            break;
    }
    
    return false;
}

bool one_card_pm_evt_handler(pm_evt_t *p_evt)
{
    // ペアリング済みである端末からの
    // 再ペアリング要求を受入れるようにする
    ble_u2f_pairing_allow_repairing(p_evt);
    
    // ペアリング情報の削除が完了したときの処理を行う
    if (ble_u2f_pairing_delete_bonds_response(p_evt)) {
        return true;
    }
    
    // ペアリングが無効になってしまった場合
    // ペアリングモードLED点滅を開始させる
    ble_u2f_t *p_u2f = one_card_get_U2F_context();
    ble_u2f_pairing_notify_unavailable(p_u2f, p_evt);
    
    return false;
}

void one_card_sleep_mode_enter(void)
{
    // FIDO U2Fで使用しているLEDを消灯
    ble_u2f_t *p_u2f = one_card_get_U2F_context();
    ble_u2f_led_light_LED(p_u2f->led_for_pairing_mode,  false);
    ble_u2f_led_light_LED(p_u2f->led_for_user_presence, false);

    // スリープモードに入る旨のログ出力
    NRF_LOG_INFO("Go to system-off mode (wakeup will cause a reset)");
}
