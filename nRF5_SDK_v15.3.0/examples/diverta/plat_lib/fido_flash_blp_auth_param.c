/* 
 * File:   fido_flash_blp_auth_param.c
 * Author: makmorit
 *
 * Created on 2019/10/22, 13:18
 */
#include "sdk_common.h"
#include "fds.h"

#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_blp_auth_param
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
// 
// BLEペリフェラルによる自動認証パラメーター管理用
//   レコードサイズ = 11 ワード
//     スキャン対象サービスUUID文字列: 9ワード（36バイト)
//     スキャン秒数: 1ワード（4バイト）
//     自動認証フラグ: 1ワード（4バイト）
static uint32_t m_blp_auth_param_record[FIDO_BLP_AUTH_PARAM_RECORD_SIZE];

static bool blp_auth_param_record_find(fds_record_desc_t *record_desc)
{
    // 作業領域の初期化
    memset(m_blp_auth_param_record, 0, FIDO_BLP_AUTH_PARAM_RECORD_SIZE * 4);

    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(FIDO_BLP_AUTH_PARAM_FILE_ID, FIDO_BLP_AUTH_PARAM_RECORD_KEY, record_desc, &ftok);
    if (ret != FDS_SUCCESS) {
        return false;
    }

    // Flash ROMに登録されているデータを読み出す
    return fido_flash_fds_record_get(record_desc, m_blp_auth_param_record);
}

bool fido_flash_blp_auth_param_read(void)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_pin_store_hash_record に読込む
    fds_record_desc_t record_desc;
    return blp_auth_param_record_find(&record_desc);
}

uint8_t *fido_flash_blp_auth_param_service_uuid_string(void)
{
    // レコード領域の先頭アドレスを戻す
    return (uint8_t *)m_blp_auth_param_record;
}

uint32_t fido_flash_blp_auth_param_service_uuid_scan_sec(void)
{
    // スキャン秒数を取得して戻す
    // （レコード領域先頭から１０ワード目）
    return m_blp_auth_param_record[9];
}

uint32_t fido_flash_blp_auth_param_service_uuid_scan_enable(void)
{
    // 自動認証有効化フラグを取得して戻す
    // （レコード領域先頭から１１ワード目）
    return m_blp_auth_param_record[10];
}

bool fido_flash_blp_auth_param_write(uint8_t *p_uuid_string, uint32_t scan_sec, uint32_t scan_enable)
{
    // Flash ROMから既存データを走査
    bool found = false;
    fds_record_desc_t record_desc;
    found = blp_auth_param_record_find(&record_desc);
    
    // スキャン対象サービスUUID文字列部 (9ワード＝36バイト)
    // NULLが引き渡された場合は、更新しないものとする
    if (p_uuid_string != NULL) {
        memcpy((uint8_t *)m_blp_auth_param_record, p_uuid_string, 36);
    }

    // スキャン秒数部 (1ワード)
    m_blp_auth_param_record[9] = scan_sec;

    // 自動認証有効化フラグ部 (1ワード)
    m_blp_auth_param_record[10] = scan_enable;

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = FIDO_BLP_AUTH_PARAM_FILE_ID;
    record.key               = FIDO_BLP_AUTH_PARAM_RECORD_KEY;
    record.data.p_data       = m_blp_auth_param_record;
    record.data.length_words = FIDO_BLP_AUTH_PARAM_RECORD_SIZE;

    ret_code_t ret;
    if (found == true) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_write returns 0x%02x ", ret);
            return false;
        }
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればシステムエラー扱い)
        NRF_LOG_ERROR("no space in flash, calling FDS GC ");
        fido_flash_fds_force_gc();
    }

    return true;
}
