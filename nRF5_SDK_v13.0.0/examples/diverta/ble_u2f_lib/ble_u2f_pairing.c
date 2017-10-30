#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_util.h"
#include "peer_manager.h"
#include "fds.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_pairing"
#include "nrf_log.h"

// Flash ROM書込み用データの一時格納領域
static fds_record_chunk_t  m_fds_record_chunks[1];
static uint32_t m_pairing_mode;
#define PAIRING_MODE     0x00000001
#define NON_PAIRING_MODE 0x00000000

// ペアリングモードを保持
static bool run_as_pairing_mode;


void ble_u2f_pairing_delete_bonds(ble_u2f_context_t *p_u2f_context)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_pairing_delete_bonds start \r\n");

    // ボンディング情報を削除
    err_code = pm_peers_delete();
    if (err_code != FDS_SUCCESS) {
        // 失敗した場合はエラーレスポンスを戻す
        NRF_LOG_ERROR("pm_peers_delete returns 0x%02x \r\n", err_code);
        ble_u2f_send_error_response(p_u2f_context, 0x01);
        return;
    }

    // ガベージコレクションを実行
    err_code = fds_gc();
    if (err_code != FDS_SUCCESS) {
        // 失敗した場合はエラーレスポンスを戻す
        NRF_LOG_ERROR("fds_gc returns 0x%02x \r\n", err_code);
        ble_u2f_send_error_response(p_u2f_context, 0x02);
        return;
    }
}

void ble_u2f_pairing_delete_bonds_response(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->id != FDS_EVT_GC) {
        // GC完了イベントでない場合はスルー
        return;
    }

    ret_code_t result = p_evt->result;
    if (result == FDS_SUCCESS) {
        // レスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_success_response(p_u2f_context);
        NRF_LOG_DEBUG("ble_u2f_pairing_delete_bonds end \r\n");

    } else {
        // エラーレスポンスを生成してU2Fクライアントに戻す
        ble_u2f_send_error_response(p_u2f_context, 0x03);
        NRF_LOG_ERROR("ble_u2f_pairing_delete_bonds abend \r\n");
    }
}

uint8_t ble_u2f_pairing_advertising_flag(void)
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

bool ble_u2f_pairing_reject_request(uint16_t ble_conn_handle, ble_evt_t *p_ble_evt)
{
    if (run_as_pairing_mode == false) {
        if (p_ble_evt->header.evt_id == BLE_GAP_EVT_SEC_PARAMS_REQUEST) {
            // ペアリングモードでない場合は、
            // ペアリング要求に応じないようにする
            NRF_LOG_ERROR("Reject pairing request from an already bonded peer. \r\n");
            sd_ble_gap_disconnect(ble_conn_handle, BLE_HCI_UNSUPPORTED_REMOTE_FEATURE);
            return true;
        }
    }
    return false;
}

bool ble_u2f_pairing_allow_repairing(pm_evt_t const *p_evt)
{
    if (p_evt->evt_id == PM_EVT_CONN_SEC_CONFIG_REQ) {
        // ペアリング済みである端末からの
        // 再ペアリング要求を受入れるようにする
        NRF_LOG_DEBUG("Accept pairing request from an already bonded peer. \r\n");
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
    m_fds_record_chunks[0].p_data       = &m_pairing_mode;
    m_fds_record_chunks[0].length_words = 1;

    fds_record_t record;
    record.file_id         = U2F_PAIRING_FILE_ID;
    record.key             = U2F_PAIRING_MODE_RECORD_KEY;
    record.data.p_chunks   = m_fds_record_chunks;
    record.data.num_chunks = 1;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(U2F_PAIRING_FILE_ID, U2F_PAIRING_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS) {
            NRF_LOG_ERROR("fds_record_update returns 0x%02x \r\n", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
        if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
            // 書込みができない場合はエラー扱いとする
            NRF_LOG_ERROR("write_pairing_mode: no space in flash \r\n");
            return false;

        } else if (ret != FDS_SUCCESS) {
            NRF_LOG_ERROR("write_pairing_mode: fds returns 0x%02x \r\n", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("fds_record_find returns 0x%02x \r\n", ret);
        return false;
    }
    
    return true;
}

void ble_u2f_pairing_change_mode(ble_u2f_context_t *p_u2f_context)
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
    
    if (fds_gc() != FDS_SUCCESS) {
        // ガベージコレクションを実行
        // NGであればエラー扱い
        return;
    }

    if (write_pairing_mode() == false) {
        // ペアリングモードをFlash ROMへ保存
        // NGであればエラー扱い
        return;
    }

    // Flash ROM保存後に
    // ble_u2f_pairing_reflect_mode_change関数が
    // 呼び出されるようにするための処理区分を設定
    p_u2f_context->command = COMMAND_CHANGE_PAIRING_MODE;
}

void ble_u2f_pairing_reflect_mode_change(ble_u2f_context_t *p_u2f_context, fds_evt_t const *const p_evt)
{
    if (p_evt->id != FDS_EVT_WRITE && p_evt->id != FDS_EVT_UPDATE) {
        // write/update完了イベントでない場合はスルー
        return;
    }
    // ソフトデバイス起動直後に行われるアドバタイジング設定処理により
    // 変更したペアリングモード設定を反映するため、システムリセットを実行
    NRF_LOG_INFO("ble_u2f_pairing_reflect_mode_change called. \r\n");
    NVIC_SystemReset();
}

static bool read_pairing_record(fds_record_desc_t *record_desc, uint32_t *data_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open returns 0x%02x \r\n", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->tl.length_words;
    memcpy(data_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x \r\n", err_code);
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
    ret = fds_record_find(U2F_PAIRING_FILE_ID, U2F_PAIRING_MODE_RECORD_KEY, &record_desc, &ftok);
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

void ble_u2f_pairing_get_mode(ble_u2f_t *p_u2f)
{
    // ペアリングモードがFlash ROMに設定されていれば
    // それを取得して設定
    run_as_pairing_mode = read_pairing_mode();

    // ペアリングモードとして動作するか否かを設定
    if (run_as_pairing_mode == true) {
        // Flash ROM上は非ペアリングモードに設定
        //   (SoftDevice再起動時に
        //   非ペアリングモードで起動させるための措置)
        // SoftDeviceが起動中は、
        // run_as_pairing_mode==trueが保持される
        m_pairing_mode = NON_PAIRING_MODE;
        write_pairing_mode();

        // 指定のLEDを点灯させる
        ble_u2f_led_light_LED(p_u2f->led_for_pairing_mode, true);
        NRF_LOG_INFO("Run as pairing mode \r\n");

    } else {
        // 指定のLEDを消灯させる
        ble_u2f_led_light_LED(p_u2f->led_for_pairing_mode, false);
        NRF_LOG_INFO("Run as non-pairing mode \r\n");
    }
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
