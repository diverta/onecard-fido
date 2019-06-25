/* 
 * File:   fido_ble_event.c
 * Author: makmorit
 *
 * Created on 2018/10/09, 11:59
 */

#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_event
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for FIDO
#include "ble_u2f.h"
#include "fido_ble_main.h"
#include "ble_u2f_command.h"
#include "ble_u2f_control_point.h"
#include "ble_u2f_util.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_status.h"
#include "fido_timer.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static void ble_u2f_on_connect(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    // U2Fクライアントとの接続が行われた時は、
    // 接続ハンドルを保持する
    p_u2f->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;

    // 共有情報を初期化する
    ble_u2f_command_initialize_context();

    // 無通信タイマーが既にスタートしている場合は停止させる
    fido_comm_interval_timer_stop();

    // アイドル時点滅処理を停止
    fido_idling_led_off();
}

static void ble_u2f_on_disconnect(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    // U2Fクライアントとの接続が切り離された時は、
    // 接続ハンドルをクリアする
    UNUSED_PARAMETER(p_ble_evt);
    p_u2f->conn_handle = BLE_CONN_HANDLE_INVALID;

    // 共有情報を消去する
    ble_u2f_command_finalize_context();

    // アイドル時点滅処理を開始
    fido_idling_led_on();
    
    // ペアリングモードをキャンセルするため、ソフトデバイスを再起動
    ble_u2f_pairing_on_disconnect();
}

static bool ble_u2f_on_write(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    if (p_evt_write->handle == p_u2f->u2f_control_point_handles.value_handle) {
        // Control Point（コマンドバッファ）の内容更新時の処理
        // コマンドバッファに入力されたリクエストデータを取得し、
        // その内容を判定し処理を実行
        ble_u2f_command_on_ble_evt_write(p_u2f, p_evt_write);
        return true;

    } else {
        // 他のBLEサービスに処理させる
        return false;
    }
}

static bool ble_u2f_on_rw_authorize_request(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    uint32_t err_code;
    ble_gatts_evt_rw_authorize_request_t  req;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

    // 書込操作以外は対象外
    req = p_ble_evt->evt.gatts_evt.params.authorize_request;
    if (req.type == BLE_GATTS_AUTHORIZE_TYPE_INVALID) {
        return false;
    }
    if (req.request.write.op != BLE_GATTS_OP_WRITE_REQ) {
        return false;
    }

    if (req.request.write.handle == p_u2f->u2f_service_revision_bitfield_handles.value_handle) {
        // U2F Service Revision Bitfieldへの書込時
        if (req.type != BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
            return false;
        }
        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;

        uint8_t bitfield = req.request.write.data[0];
        if (bitfield == 0x80) {
            // 0x80指定時は書込許可
            auth_reply.params.write.update = 1;
            auth_reply.params.write.offset = 0;
            auth_reply.params.write.len = req.request.write.len;
            auth_reply.params.write.p_data = req.request.write.data;
            auth_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;

        } else {
            // 0x80以外は書込不許可
            auth_reply.params.write.gatt_status = BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
        }

        // レスポンス実行
        err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle, &auth_reply);
        APP_ERROR_CHECK(err_code);
        return true;

    } else {
        // 他のBLEサービスに処理させる
        return false;
    }
}

bool fido_ble_evt_handler(ble_evt_t *p_ble_evt, void *p_context)
{
    UNUSED_PARAMETER(p_context);
    if (p_ble_evt == NULL) {
        return false;
    }
    NRF_LOG_DEBUG("BLE event id=0x%02x", p_ble_evt->header.evt_id);
    
    bool ret = false;
    ble_u2f_t *p_u2f = fido_ble_get_U2F_context();
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

        case BLE_GATTS_EVT_WRITE:
            ret = ble_u2f_on_write(p_u2f, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            ret = ble_u2f_on_rw_authorize_request(p_u2f, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            ble_u2f_status_on_tx_complete(p_u2f);
            break;

        default:
            break;
    }
    
    return ret;
}

bool fido_ble_pm_evt_handler(pm_evt_t *p_evt)
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
    ble_u2f_pairing_notify_unavailable(p_evt);
    
    return false;
}

void fido_ble_sleep_mode_enter(void)
{
    // FIDO U2Fで使用しているLEDを消灯
    fido_led_light_all(false);
}

void fido_ble_on_process_timedout(void)
{
    ble_u2f_t *p_u2f = fido_ble_get_U2F_context();
    if (p_u2f == NULL) {
        return;
    }

    // 直近のレスポンスから10秒を経過した場合、
    // nRF52から強制的にBLEコネクションを切断
    NRF_LOG_DEBUG("Communication interval timed out: received %d frames", ble_u2f_control_point_receive_frame_count());
    sd_ble_gap_disconnect(p_u2f->conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
}
