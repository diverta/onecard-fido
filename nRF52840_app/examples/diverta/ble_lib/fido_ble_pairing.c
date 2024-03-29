#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "peer_manager.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"

// for flushing logs
#include "nrf_log_ctrl.h"
#include "nrf_delay.h"

#include "fido_ble_define.h"
#include "fido_ble_event.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"

#include "application_init.h"
#include "fido_ble_service.h"
#include "fido_ble_service_define.h"
#include "fido_flash_pairing_mode.h"
#include "fido_timer_plat.h"

// for fido_board_delay_ms
#include "fido_board.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_pairing
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// 直近レスポンスからの経過秒数監視するためのタイムアウト（ミリ秒単位）
#define COMMUNICATION_INTERVAL_MSEC 10000

// ペアリングモードを保持
static bool run_as_pairing_mode;

// ペアリング完了フラグ（ペアリングモードで、ペアリング完了時にtrueが設定される）
static bool pairing_completed;

// ペアリングモード変更中の旨を保持
static bool change_pairing_mode = false;

//
// USB接続が検出されなかった場合
// スリープ状態に遷移させるためのフラグ。
// Flash ROMにペアリングモードレコードが
// 存在していない場合に true を設定
//
static bool sleep_after_boot;

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

//
// ペアリングモードの場合、アドバタイズデータに
// サービスデータフィールドを追加設定
//
static uint8_array_t service_flag_data;
static ble_advdata_service_data_t service_data;

// サービスデータフィールドのビット内容
//   7: Device is in pairing mode.
static uint8_t service_flag[] = {0x80};

void fido_ble_pairing_add_service_data_field(void *p_init)
{
    if (run_as_pairing_mode == false) {
        return;
    }

    // サービスデータフィールドの設定
    service_flag_data.p_data = service_flag;
    service_flag_data.size = (uint16_t)1;

    // 対応するサービスUUID（FIDO）の設定
    service_data.service_uuid = BLE_UUID_U2F_SERVICE;
    service_data.data = service_flag_data;

    // サービスデータフィールドをアドバタイズデータに設定
    ble_advertising_init_t *init = (ble_advertising_init_t *)p_init;
    init->advdata.p_service_data_array = &service_data;
    init->advdata.service_data_count = 1;
}

bool fido_ble_pairing_allow_repairing(void const *pm_evt)
{
    pm_evt_t const *p_evt = (pm_evt_t const *)pm_evt;
    if (run_as_pairing_mode) {
        if (p_evt->evt_id == PM_EVT_CONN_SEC_CONFIG_REQ) {
            // ペアリング済みである端末からの
            // 再ペアリング要求を受入れるようにする
            NRF_LOG_DEBUG("Accept pairing request from an already bonded peer. ");
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
            return true;
        }
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
        fido_board_system_reset();
    }
}

bool fido_ble_pairing_get_peer_count(uint8_t *p_count)
{
    // ペアリング情報（IRK）を含む peer_id の配列を抽出
    pm_peer_id_t peer_list[10];
    uint32_t list_size = sizeof(peer_list) / sizeof(pm_peer_id_t);
    ret_code_t ret_code = pm_peer_id_list(peer_list, &list_size, PM_PEER_ID_INVALID, PM_PEER_ID_LIST_SKIP_NO_IRK);
    if (ret_code != NRF_SUCCESS) {
        *p_count = 0;
        return false;
    }

#if LOG_DEBUG_PEER_ID_LIST
    for (uint8_t i = 0; i < list_size; i++) {
        NRF_LOG_DEBUG("fido_ble_pairing_get_mode: peer_id[%d]=%d", i, peer_list[i]);
    }
#endif

    // ペアリング情報（IRK）を含む peer_id の数を戻す
    *p_count = (uint8_t)list_size;
    return true;
}

void fido_ble_pairing_get_mode(void)
{
    // ペアリング情報が存在しない場合は、優先してペアリングモードとする
    bool no_peer = true;

    // ペアリング情報の有無を照会
    uint8_t peer_count;
    if (fido_ble_pairing_get_peer_count(&peer_count) && (peer_count > 0)) {
        // ペアリング情報が１件以上存在すれば、非ペアリングモードとする
        NRF_LOG_INFO("Already bonded peer is exist (count=%d).", peer_count);
        no_peer = false;
    } else {
        NRF_LOG_INFO("Already bonded peer is not exist.");
    }

    // ペアリングモードがFlash ROMに設定されていれば
    // それを取得して設定
    bool exist;
    run_as_pairing_mode = fido_flash_pairing_mode_flag(&exist) | no_peer;
    NRF_LOG_INFO("Run as %s mode",
        run_as_pairing_mode ? "pairing" : "non-pairing");

    // USB接続が検出されなかった場合
    // スリープ状態に遷移させるためのフラグを設定。
    // Flash ROMにペアリングモードレコードが
    // 存在していない場合は true
    sleep_after_boot = (exist == false);

    // ボタン長押しでペアリングモードに遷移させる場合は
    // このタイミングで黄色LEDを点灯させる
    if (fido_flash_pairing_mode_flag_get()) {
        fido_status_indicator_pairing_mode();
    }

    // Flash ROM上は非ペアリングモードに設定
    //   (SoftDevice再起動時に
    //   非ペアリングモードで起動させるための措置)
    // SoftDeviceが起動中は、
    // run_as_pairing_mode==trueが保持される
    fido_flash_pairing_mode_flag_clear();
    
    // ペアリング完了フラグを初期化
    pairing_completed = false;
}

void fido_ble_pairing_on_evt_auth_status(void *ble_evt)
{
    // LESCペアリング完了時のステータスを確認
    ble_evt_t *p_ble_evt = (ble_evt_t *)ble_evt;
    uint8_t auth_status = p_ble_evt->evt.gap_evt.params.auth_status.auth_status;
    NRF_LOG_INFO("Authorization status: 0x%02x ", auth_status);

    // ペアリング成功時はペアリングモードをキャンセル
    // （ペアリングキャンセルのためのソフトデバイス再起動は、disconnect時に実行される）
    if (run_as_pairing_mode == true && auth_status == BLE_GAP_SEC_STATUS_SUCCESS) {
        NRF_LOG_INFO("Pairing completed with success ");
        pairing_completed = true;
        
        // ペアリング先から切断されない可能性があるため、
        // 無通信タイムアウトのタイマー（10秒）をスタートさせる
        fido_comm_interval_timer_start(COMMUNICATION_INTERVAL_MSEC, fido_ble_on_process_timedout);
    }
    
    // 非ペアリングモードでペアリング要求があった場合も
    // ペアリング先から切断されない可能性があるため、
    // 無通信タイムアウトのタイマー（10秒）をスタートさせる
    if (auth_status == BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP) {
        NRF_LOG_INFO("Pairing rejected");
        fido_comm_interval_timer_start(COMMUNICATION_INTERVAL_MSEC, fido_ble_on_process_timedout);
    }
}

void fido_ble_pairing_on_disconnect(void)
{
    if (run_as_pairing_mode == true && pairing_completed == true) {
        // 無通信タイマーが既にスタートしている場合は停止させる
        fido_comm_interval_timer_stop();

        // ペアリングモードをキャンセルするため、ソフトデバイスを再起動
        // （再起動後は非ペアリングモードで起動し、ディスカバリーができないようになる）
        fido_board_delay_ms(500);
        fido_board_system_reset();
    }
}

bool fido_ble_pairing_mode_get(void)
{
    // ペアリングモードであればtrueを戻す
    return run_as_pairing_mode;
}

//
// ペアリングモードのリセット
//
void fido_ble_pairing_reset(void)
{
    // ペアリングモードレコードをFlash ROMから削除
    //   システムのリスタート時、
    //   BLEアイドル状態に遷移するのを抑止するための措置
    fido_flash_pairing_mode_flag_reset();
}

void fido_ble_pairing_flash_deleted(void)
{
    NRF_LOG_DEBUG("Pairing mode record deleted");
    application_init_ble_pairing_has_reset();
}

bool fido_ble_pairing_sleep_after_boot_mode(void)
{
    // USB接続が検出されなかった場合
    // スリープ状態に遷移させるためのフラグ。
    // Flash ROMにペアリングモードレコードが
    // 存在していない場合は true
    return sleep_after_boot;
}

//
// ペアリング解除関連
//
bool fido_ble_pairing_get_peer_id(uint16_t *p_peer_id) 
{
    // コネクションハンドルからpeer_idを取得
    ble_u2f_t *p_u2f = (ble_u2f_t *)fido_ble_get_U2F_context();
    uint16_t conn_handle = p_u2f->conn_handle;
    ret_code_t ret = pm_peer_id_get(conn_handle, p_peer_id);
    if (ret == NRF_SUCCESS) {
        NRF_LOG_DEBUG("Connected peer id=0x%04x", *p_peer_id);
        return true;

    } else if (ret == NRF_ERROR_NULL) {
        NRF_LOG_DEBUG("peer id is not exist");
        return false;

    } else {
        NRF_LOG_ERROR("pm_peer_id_get returns %d", ret);
        return false;
    }
}

bool fido_ble_pairing_delete_peer_id(uint16_t peer_id)
{
    // コネクションハンドルからpeer_idを取得
    ret_code_t ret = pm_peer_delete(peer_id);
    if (ret == NRF_SUCCESS) {
        return true;

    } else if (ret == NRF_ERROR_INVALID_PARAM) {
        NRF_LOG_DEBUG("peer id (0x%04x) is not valid", peer_id);
        return false;

    } else {
        NRF_LOG_ERROR("pm_peer_id_get returns %d", ret);
        return false;
    }
}

bool fido_ble_pairing_peer_deleted(void *pm_evt)
{
    pm_evt_t *p_evt = (pm_evt_t *)pm_evt;
    pm_evt_id_t evt_id = p_evt->evt_id;
    pm_peer_id_t peer_id = p_evt->peer_id;

    if (evt_id == PM_EVT_PEER_DELETE_SUCCEEDED) {
        // ペアリング情報削除成功時
        fido_ble_unpairing_done(true, peer_id);
        return true;
    }

    if (evt_id == PM_EVT_PEER_DELETE_FAILED) {
        // ペアリング情報削除失敗時
        fido_ble_unpairing_done(false, peer_id);
        return true;
    }

    return false;
}
