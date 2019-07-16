#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "peer_manager.h"
#include "fds.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"

#include "fido_ble_command.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "u2f.h"

#include "fido_flash.h"
#include "fido_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_pairing
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// ペアリングモードを保持
static bool run_as_pairing_mode;

// ペアリング完了フラグ（ペアリングモードで、ペアリング完了時にtrueが設定される）
static bool pairing_completed;

// ペアリングモード変更中の旨を保持
static bool change_pairing_mode = false;

void fido_ble_pairing_delete_bonds(void)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_pairing_delete_bonds start ");

    // ボンディング情報を削除
    err_code = pm_peers_delete();
    if (err_code != FDS_SUCCESS) {
        // 失敗した場合はエラーレスポンスを戻す
        NRF_LOG_ERROR("pm_peers_delete returns 0x%02x ", err_code);
        uint8_t cmd = fido_ble_receive_header()->CMD;
        fido_ble_send_status_word(cmd, 0x9101);
        return;
    }
}

bool fido_ble_pairing_delete_bonds_response(pm_evt_t const *p_evt)
{
    uint8_t cmd = fido_ble_receive_header()->CMD;

    // pm_peers_deleteが完了したときの処理。
    //   PM_EVT_PEERS_DELETE_SUCCEEDED、または
    //   PM_EVT_PEERS_DELETE_FAILEDの
    //   いずれかのイベントが発生する
    // 成功or失敗の旨のレスポンスを生成し、U2Fクライアントに戻す
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_SUCCEEDED) {
        fido_ble_send_status_word(cmd, U2F_SW_NO_ERROR);
        NRF_LOG_DEBUG("ble_u2f_pairing_delete_bonds end ");
        return true;
    }
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_FAILED) {
        fido_ble_send_status_word(cmd, 0x9102);
        NRF_LOG_ERROR("ble_u2f_pairing_delete_bonds abend: Peer manager event=%d ", p_evt->evt_id);
        return true;
    }
    
    return false;
}

uint8_t fido_ble_pairing_advertising_flag(void)
{
    uint8_t advdata_flags;

    if (run_as_pairing_mode == false) {
        // ペアリングモードでない場合は、
        // ディスカバリーができないよう設定
        advdata_flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    } else {
        advdata_flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    }

    return advdata_flags;
}

bool fido_ble_pairing_reject_request(ble_evt_t const *p_ble_evt)
{
    if (run_as_pairing_mode == false) {
        if (p_ble_evt->header.evt_id == BLE_GAP_EVT_SEC_PARAMS_REQUEST) {
            // ペアリングモードでない場合は、
            // ペアリング要求に応じないようにする
            NRF_LOG_ERROR("Reject pairing request from an already bonded peer. ");
            uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            ret_code_t code = sd_ble_gap_sec_params_reply(conn_handle, BLE_GAP_SEC_STATUS_UNSPECIFIED, NULL, NULL);
            APP_ERROR_CHECK(code);
            // ペアリングモードLED点滅を開始し、
            // 再度ペアリングが必要であることを通知
            fido_status_indicator_pairing_fail();
            return true;
        }
    }
    return false;
}

static void ble_evt_handler(ble_evt_t const *p_ble_evt, void * p_context)
{
    // ペアリングモードでない場合は、
    // ペアリング要求に応じないようにする
    fido_ble_pairing_reject_request(p_ble_evt);
}

NRF_SDH_BLE_OBSERVER(m_ble_evt_observer, BLE_CONN_STATE_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

bool fido_ble_pairing_allow_repairing(pm_evt_t const *p_evt)
{
    if (run_as_pairing_mode == false) {
        // ペアリングモードでない場合は何もしない
        return false;
    }
    if (p_evt->evt_id == PM_EVT_CONN_SEC_CONFIG_REQ) {
        // ペアリング済みである端末からの
        // 再ペアリング要求を受入れるようにする
        NRF_LOG_DEBUG("Accept pairing request from an already bonded peer. ");
        pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        return true;
    }
    return false;
}

void fido_ble_pairing_change_mode(void)
{
    // ペアリングモードをFlash ROMへ保存
    // (fds_record_update/writeまたはfds_gcが実行される)
    if (run_as_pairing_mode == false) {
        // 非ペアリングモードの場合は
        // ペアリングモードに移行させる
        fido_flash_pairing_mode_flag_set();
    } else {
        // ペアリングモードの場合は
        // 非ペアリングモードに移行させる
        fido_flash_pairing_mode_flag_clear();
    }

    // fds_gc完了後に
    // ble_u2f_pairing_reflect_mode_change関数が
    // 呼び出されるようにするための処理区分を設定
    change_pairing_mode = true;
}

static bool is_pairing_mode_changing(void)
{
    if (change_pairing_mode == false) {
        return false;
    }
    change_pairing_mode = false;
    return true;
}

void fido_ble_pairing_flash_failed(void)
{
    // Flash ROM処理でエラーが発生時
    if (is_pairing_mode_changing()) {
        NRF_LOG_ERROR("ble_u2f_pairing_change_mode abend");
    }
}

void fido_ble_pairing_flash_gc_done(void)
{
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // エラーメッセージを出力
    if (is_pairing_mode_changing()) {
        NRF_LOG_ERROR("ble_u2f_pairing_reflect_mode_change abend: FDS GC done ");
    }
}

void fido_ble_pairing_flash_updated(void)
{
    if (is_pairing_mode_changing()) {
        // ble_u2f_pairing_change_modeにより実行した
        // fds_record_update/writeが正常完了の場合、
        // ソフトデバイス起動直後に行われるアドバタイジング設定処理により
        // 変更したペアリングモード設定を反映するため、システムリセットを実行
        NVIC_SystemReset();
    }
}

void fido_ble_pairing_get_mode(void)
{
    // ペアリングモードがFlash ROMに設定されていれば
    // それを取得して設定
    run_as_pairing_mode = fido_flash_pairing_mode_flag();
    NRF_LOG_INFO("Run as %s mode",
        run_as_pairing_mode ? "pairing" : "non-pairing");

    // Flash ROM上は非ペアリングモードに設定
    //   (SoftDevice再起動時に
    //   非ペアリングモードで起動させるための措置)
    // SoftDeviceが起動中は、
    // run_as_pairing_mode==trueが保持される
    fido_flash_pairing_mode_flag_clear();
    
    // ペアリング完了フラグを初期化
    pairing_completed = false;
}

void fido_ble_pairing_on_evt_auth_status(ble_evt_t * p_ble_evt)
{
    // LESCペアリング完了時のステータスを確認
    uint8_t auth_status = p_ble_evt->evt.gap_evt.params.auth_status.auth_status;
    NRF_LOG_INFO("Authorization status: 0x%02x ", auth_status);

    // ペアリング成功時はペアリングモードをキャンセル
    // （ペアリングキャンセルのためのソフトデバイス再起動は、disconnect時に実行される）
    if (run_as_pairing_mode == true && auth_status == BLE_GAP_SEC_STATUS_SUCCESS) {
        NRF_LOG_INFO("Pairing completed with success ");
        pairing_completed = true;
        
        // ペアリング先から切断されない可能性があるため、
        // 無通信タイムアウトのタイマー（10秒）をスタートさせる
        fido_comm_interval_timer_start();
    }
    
    // 非ペアリングモードでペアリング要求があった場合も
    // ペアリング先から切断されない可能性があるため、
    // 無通信タイムアウトのタイマー（10秒）をスタートさせる
    if (auth_status == BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP) {
        NRF_LOG_INFO("Pairing rejected");
        fido_comm_interval_timer_start();
    }
}

void fido_ble_pairing_on_disconnect(void)
{
    if (run_as_pairing_mode == true && pairing_completed == true) {
        // 無通信タイマーが既にスタートしている場合は停止させる
        fido_comm_interval_timer_stop();

        // ペアリングモードをキャンセルするため、ソフトデバイスを再起動
        // （再起動後は非ペアリングモードで起動し、ディスカバリーができないようになる）
        NRF_LOG_DEBUG("ble_u2f_pairing_on_disconnect called. ");
        NVIC_SystemReset();
    }
}

void fido_ble_pairing_notify_unavailable(pm_evt_t const *p_evt)
{
    if (run_as_pairing_mode == true) {
        // ペアリングモードの場合は何もしない
        return;
    }
    
    if (p_evt->evt_id == PM_EVT_CONN_SEC_FAILED) {
        // ペアリングが無効である場合、ペアリングモードLED点滅を開始
        fido_status_indicator_pairing_fail();
    } else if (p_evt->evt_id == PM_EVT_CONN_SEC_SUCCEEDED) {
        // ペアリングが有効である場合、LED制御を
        // ペアリングモード-->非ペアリングモードに変更
        fido_status_indicator_no_idle();
    }
}

bool fido_ble_pairing_mode_get(void)
{
    // ペアリングモードであればtrueを戻す
    return run_as_pairing_mode;
}
