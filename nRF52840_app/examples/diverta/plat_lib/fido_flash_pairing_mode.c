/* 
 * File:   fido_flash_pairing_mode.c
 * Author: makmorit
 *
 * Created on 2019/07/08, 9:32
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "fido_flash_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_pairing_mode
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
static fds_record_t m_fds_record;
static uint32_t m_pairing_mode;

// Flash ROMに書き込まれる内容の定義
#define PAIRING_MODE     0x00000001
#define NON_PAIRING_MODE 0x00000000

static bool delete_pairing_mode(void)
{
    // Flash ROM領域から削除
    ret_code_t err_code = fds_file_delete(FIDO_PAIRING_MODE_FILE_ID);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x ", err_code);
        return false;
    }
    return true;
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
    if (ret == NRF_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &m_fds_record);
        if (ret != NRF_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != NRF_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("write_pairing_mode: fds_record_find returns 0x%02x ", ret);
        return false;
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればシステムエラー扱い)
        NRF_LOG_ERROR("no space in flash, calling FDS GC ");
        fido_flash_fds_force_gc();
    }
    
    return true;
}

static bool read_pairing_record(fds_record_desc_t *record_desc, uint32_t *data_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;

    ret_code_t err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(data_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x ", err_code);
        return false;	
    }
    return true;
}

static bool read_pairing_mode(bool *p_exist)
{
    // 非ペアリングモードで初期化
    m_pairing_mode = NON_PAIRING_MODE;
    *p_exist = false;

    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(FIDO_PAIRING_MODE_FILE_ID, FIDO_PAIRING_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == NRF_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        *p_exist = true;
        return read_pairing_record(&record_desc, &m_pairing_mode);

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // レコードが存在しないときは、非ペアリングモードとする
        NRF_LOG_DEBUG("Pairing mode record not found");
        return false;

    } else {
        // その他エラー発生時
        NRF_LOG_DEBUG("read_pairing_mode: fds_record_find returns 0x%02x ", ret);
        return false;
    }
}

bool fido_flash_pairing_mode_flag(bool *p_exist)
{
    if (read_pairing_mode(p_exist)) {
        // ペアリングモードレコードの設定内容から
        // ペアリングモードかどうかを取得
        return (m_pairing_mode == PAIRING_MODE);

    } else {
        // レコードが存在しないときや
        // その他エラー発生時は非ペアリングモード
        return false;
    }
}

bool fido_flash_pairing_mode_flag_get(void)
{
    // ペアリングモードレコードがペアリングモードに
    // 設定されている場合は true
    return m_pairing_mode == PAIRING_MODE;
}

void fido_flash_pairing_mode_flag_clear(void)
{
    m_pairing_mode = NON_PAIRING_MODE;
    write_pairing_mode();
}

void fido_flash_pairing_mode_flag_set(void)
{
    m_pairing_mode = PAIRING_MODE;
    write_pairing_mode();
}

void fido_flash_pairing_mode_flag_reset(void)
{
    m_pairing_mode = NON_PAIRING_MODE;
    delete_pairing_mode();
}

