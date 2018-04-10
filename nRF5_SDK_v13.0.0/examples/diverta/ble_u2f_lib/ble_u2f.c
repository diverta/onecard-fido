#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_util.h"

// 無通信タイマー
#include "ble_u2f_comm_interval_timer.h"

// キープアライブタイマー
#include "ble_u2f_user_presence.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f"
#include "nrf_log.h"

static void ble_u2f_on_connect(ble_u2f_t * p_u2f, ble_evt_t *p_ble_evt)
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

static void ble_u2f_on_disconnect(ble_u2f_t * p_u2f, ble_evt_t *p_ble_evt)
{
    // U2Fクライアントとの接続が切り離された時は、
    // 接続ハンドルをクリアする
    UNUSED_PARAMETER(p_ble_evt);
    p_u2f->conn_handle = BLE_CONN_HANDLE_INVALID;

    // 共有情報を消去する
    ble_u2f_command_finalize_context();

    // FIDO機能実行中LEDを消灯
    ble_u2f_led_light_LED(p_u2f->led_for_processing_fido, false);
}


static bool ble_u2f_on_write(ble_u2f_t *p_u2f, ble_evt_t *p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_u2f->u2f_status_handles.cccd_handle)
     && (p_evt_write->len == 2)) {
        // U2F Statusについて、CCCD（2バイト）が書き込まれた場合は
        // Notificationステータスを更新する。
        if (ble_srv_is_notification_enabled(p_evt_write->data)) {
            p_u2f->is_notification_enabled = true;
            NRF_LOG_DEBUG("on_cccd_write: Notification status changed to enabled \r\n");
        } else {
            p_u2f->is_notification_enabled = false;
            NRF_LOG_DEBUG("on_cccd_write: Notification status changed to disabled \r\n");
        }
        return true;

    } else if (p_evt_write->handle == p_u2f->u2f_control_point_handles.value_handle) {
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


bool ble_u2f_on_ble_evt(ble_u2f_t * p_u2f, ble_evt_t * p_ble_evt)
{
    if ((p_u2f == NULL) || (p_ble_evt == NULL)) {
        return false;
    }

    bool ret = false;
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            ble_u2f_on_connect(p_u2f, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            ble_u2f_on_disconnect(p_u2f, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            ret = ble_u2f_on_write(p_u2f, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            ret = ble_u2f_on_rw_authorize_request(p_u2f, p_ble_evt);
            break;

        default:
            break;
    }
    
    return ret;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
