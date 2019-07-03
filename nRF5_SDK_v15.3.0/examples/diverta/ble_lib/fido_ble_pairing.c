#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "fido_ble_command.h"
#include "fido_flash.h"
#include "peer_manager.h"
#include "fds.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "fido_ble_receive.h"
#include "fido_ble_send.h"
#include "fido_timer.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Flash ROM書込み用データの一時格納領域
static fds_record_t m_fds_record;
static uint32_t m_pairing_mode;
#define PAIRING_MODE     0x00000001
#define NON_PAIRING_MODE 0x00000000

// ペアリングモードを保持
static bool run_as_pairing_mode;

// ペアリング完了フラグ（ペアリングモードで、ペアリング完了時にtrueが設定される）
static bool pairing_completed;

// ペアリングモード変更中の旨を保持
static bool change_pairing_mode = false;

void fido_ble_pairing_delete_bonds(void)
{
    ret_code_t err_code;
    fido_log_debug("ble_u2f_pairing_delete_bonds start ");

    // ボンディング情報を削除
    err_code = pm_peers_delete();
    if (err_code != FDS_SUCCESS) {
        // 失敗した場合はエラーレスポンスを戻す
        fido_log_error("pm_peers_delete returns 0x%02x ", err_code);
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
        fido_log_debug("ble_u2f_pairing_delete_bonds end ");
        return true;
    }
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_FAILED) {
        fido_ble_send_status_word(cmd, 0x9102);
        fido_log_error("ble_u2f_pairing_delete_bonds abend: Peer manager event=%d ", p_evt->evt_id);
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
            fido_log_error("Reject pairing request from an already bonded peer. ");
            uint16_t conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            ret_code_t code = sd_ble_gap_sec_params_reply(conn_handle, BLE_GAP_SEC_STATUS_UNSPECIFIED, NULL, NULL);
            APP_ERROR_CHECK(code);
            // ペアリングモードLED点滅を開始し、
            // 再度ペアリングが必要であることを通知
            fido_processing_led_on(LED_LIGHT_FOR_PAIRING_MODE, LED_ON_OFF_INTERVAL_MSEC);
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
        fido_log_debug("Accept pairing request from an already bonded peer. ");
        pm_conn_sec_config_t conn_sec_config = {.allow_repairing = true};
        pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        return true;
    }
    return false;
}

static bool write_pairing_mode(void)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    m_fds_record.data.p_data       = &m_pairing_mode;
    m_fds_record.data.length_words = 1;
    m_fds_record.file_id           = FIDO_PAIRING_MODE_FILE_ID;
    m_fds_record.key               = FIDO_PAIRING_MODE_RECORD_KEY;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(FIDO_PAIRING_MODE_FILE_ID, FIDO_PAIRING_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            fido_log_error("write_pairing_mode: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            fido_log_error("write_pairing_mode: fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        fido_log_debug("write_pairing_mode: fds_record_find returns 0x%02x ", ret);
        return false;
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        fido_log_error("write_pairing_mode: no space in flash, calling FDS GC ");
        if (fido_flash_force_fdc_gc() == false) {
            return false;
        }
    }
    
    return true;
}

void fido_ble_pairing_change_mode(void)
{
    if (run_as_pairing_mode == false) {
        // 非ペアリングモードの場合は
        // ペアリングモードに移行させる
        m_pairing_mode = PAIRING_MODE;
    } else {
        // ペアリングモードの場合は
        // 非ペアリングモードに移行させる
        m_pairing_mode = NON_PAIRING_MODE;
    }
    
    // ペアリングモードをFlash ROMへ保存
    // (fds_record_update/writeまたはfds_gcが実行される)
    write_pairing_mode();
    
    // fds_gc完了後に
    // ble_u2f_pairing_reflect_mode_change関数が
    // 呼び出されるようにするための処理区分を設定
    change_pairing_mode = true;
}

void fido_ble_pairing_reflect_mode_change(void const *p_evt)
{
    if (change_pairing_mode == false) {
        return;
    }
    change_pairing_mode = false;

    fido_flash_event_t *evt = (fido_flash_event_t *)p_evt;
    if (evt->result == false) {
        // FDS処理でエラーが発生時は以降の処理を行わない
        fido_log_error("ble_u2f_pairing_change_mode abend");
        return;
    }

    if (evt->write_update && evt->pairing_mode_write) {
        // ble_u2f_pairing_change_modeにより実行した
        // fds_record_update/writeが正常完了の場合、
        // ソフトデバイス起動直後に行われるアドバタイジング設定処理により
        // 変更したペアリングモード設定を反映するため、システムリセットを実行
        NVIC_SystemReset();

    } else if (evt->gc) {
        // FDSリソース不足解消のためGCが実行された場合は、
        // エラーメッセージを出力
        fido_log_error("ble_u2f_pairing_reflect_mode_change abend: FDS GC done ");
    }
}

static bool read_pairing_record(fds_record_desc_t *record_desc, uint32_t *data_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        fido_log_error("fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(data_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        fido_log_error("fds_record_close returns 0x%02x ", err_code);
        return false;	
    }
    return true;
}

static bool read_pairing_mode(void)
{
    ret_code_t ret;

    // 非ペアリングモードで初期化
    m_pairing_mode = 0;
    
    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(FIDO_PAIRING_MODE_FILE_ID, FIDO_PAIRING_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        if (read_pairing_record(&record_desc, &m_pairing_mode) == false) {
            // データ格納失敗時は非ペアリングモード
            return false;
        } else {
            if (m_pairing_mode == PAIRING_MODE) {
                // 格納されたデータが
                // PAIRING_MODEの場合はペアリングモード
                return true;
            } else {
                return false;
            }
        }
    } else {
        // レコードが存在しないときや
        // その他エラー発生時は非ペアリングモード
        return false;
    }
}

void fido_ble_pairing_get_mode(void)
{
    // ペアリングモードがFlash ROMに設定されていれば
    // それを取得して設定
    run_as_pairing_mode = read_pairing_mode();
    fido_log_info("Run as %s mode",
        run_as_pairing_mode ? "pairing" : "non-pairing");

    // Flash ROM上は非ペアリングモードに設定
    //   (SoftDevice再起動時に
    //   非ペアリングモードで起動させるための措置)
    // SoftDeviceが起動中は、
    // run_as_pairing_mode==trueが保持される
    m_pairing_mode = NON_PAIRING_MODE;
    write_pairing_mode();
    
    // ペアリング完了フラグを初期化
    pairing_completed = false;
}

void fido_ble_pairing_on_evt_auth_status(ble_evt_t * p_ble_evt)
{
    // LESCペアリング完了時のステータスを確認
    uint8_t auth_status = p_ble_evt->evt.gap_evt.params.auth_status.auth_status;
    fido_log_info("Authorization status: 0x%02x ", auth_status);

    // ペアリング成功時はペアリングモードをキャンセル
    // （ペアリングキャンセルのためのソフトデバイス再起動は、disconnect時に実行される）
    if (run_as_pairing_mode == true && auth_status == BLE_GAP_SEC_STATUS_SUCCESS) {
        fido_log_info("Pairing completed with success ");
        pairing_completed = true;
        
        // ペアリング先から切断されない可能性があるため、
        // 無通信タイムアウトのタイマー（10秒）をスタートさせる
        fido_comm_interval_timer_start();
    }
    
    // 非ペアリングモードでペアリング要求があった場合も
    // ペアリング先から切断されない可能性があるため、
    // 無通信タイムアウトのタイマー（10秒）をスタートさせる
    if (auth_status == BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP) {
        fido_log_info("Pairing rejected");
        fido_comm_interval_timer_start();
    }
}

void fido_ble_pairing_on_disconnect(void)
{
    // ペアリングモードをキャンセルするため、ソフトデバイスを再起動
    // （再起動後は非ペアリングモードで起動し、ディスカバリーができないようになる）
    if (run_as_pairing_mode == true && pairing_completed == true) {
        fido_log_debug("ble_u2f_pairing_on_disconnect called. ");
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
        fido_processing_led_on(LED_LIGHT_FOR_PAIRING_MODE, LED_ON_OFF_INTERVAL_MSEC);
    } else if (p_evt->evt_id == PM_EVT_CONN_SEC_SUCCEEDED) {
        // ペアリングが有効である場合、ペアリングモードLED点滅を停止
        fido_processing_led_off();
    }
}

bool fido_ble_pairing_mode_get(void)
{
    // ペアリングモードであればtrueを戻す
    return run_as_pairing_mode;
}
