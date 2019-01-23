#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include "ble_u2f_securekey.h"
#include "fido_flash.h"
#include "fds.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
static fds_record_t m_fds_record;
static uint32_t m_token_counter_record_buffer[10];
static uint32_t m_token_counter;
static uint32_t m_reserve_word;

// 鍵・証明書データ読込用の作業領域（固定長）
static uint32_t skey_cert_data[SKEY_CERT_WORD_NUM];

uint32_t *fido_flash_skey_cert_data(void)
{
    return skey_cert_data;
}

bool fido_flash_force_fdc_gc(void)
{
    ret_code_t err_code;

    // FDSガベージコレクションを強制実行
    err_code = fds_gc();
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_gc returns 0x%02x ", err_code);
        APP_ERROR_CHECK(err_code);
    }

    return true;
}

bool fido_flash_skey_cert_delete(void)
{
    ret_code_t err_code;

    // 秘密鍵／証明書をFlash ROM領域から削除
    err_code = fds_file_delete(FIDO_SKEY_CERT_FILE_ID);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x ", err_code);
        return false;
    }

    return true;
}


static bool keydata_record_get(fds_record_desc_t *record_desc, uint32_t *keydata_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(keydata_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x ", err_code);
        return false;	
    }
    return true;
}


bool fido_flash_skey_cert_read(void)
{
    // 確保領域は0xff（Flash ROM未書込状態）で初期化
    memset(skey_cert_data, 0xff, sizeof(uint32_t) * SKEY_CERT_WORD_NUM);

    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(FIDO_SKEY_CERT_FILE_ID, FIDO_SKEY_CERT_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        if (keydata_record_get(&record_desc, skey_cert_data) == false) {
            return false;
        }

    } else if (ret != FDS_ERR_NOT_FOUND) {
        // レコードが存在しないときはOK
        // それ以外の場合はエラー終了
        NRF_LOG_ERROR("fds_record_find returns 0x%02x ", ret);
        return false;
    }
    
    return true;
}

bool fido_flash_skey_cert_available(void)
{
    // 一時領域が初期状態であればunavailableと判定
    // (初期状態=確保領域の全ワードが0xffffffff)
    for (uint16_t i = 0; i < SKEY_CERT_WORD_NUM; i++) {
        if (skey_cert_data[i] != 0xffffffff) {
            return true;
        }
    }
    return false;
}


bool fido_flash_skey_cert_write(void)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    m_fds_record.data.p_data       = skey_cert_data;
    m_fds_record.data.length_words = SKEY_CERT_WORD_NUM;
    m_fds_record.file_id         = FIDO_SKEY_CERT_FILE_ID;
    m_fds_record.key             = FIDO_SKEY_CERT_RECORD_KEY;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(FIDO_SKEY_CERT_FILE_ID, FIDO_SKEY_CERT_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fido_flash_skey_cert_write: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fido_flash_skey_cert_write: fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("fido_flash_skey_cert_write: fds_record_find returns 0x%02x ", ret);
        return false;
    }
    
    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        NRF_LOG_ERROR("fido_flash_skey_cert_write: no space in flash, calling FDS GC ");
        if (fido_flash_force_fdc_gc() == false) {
            return false;
        }
    }
    
    return true;
}

bool fido_flash_token_counter_delete(void)
{
    //
    // TODO: 後日正式実装します。
    //
    return true;
}

static bool token_counter_record_get(fds_record_desc_t *record_desc, uint32_t *token_counter_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(token_counter_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x ", err_code);
        return false;	
    }

    return true;
}

static bool token_counter_record_find(uint8_t *p_appid_hash, fds_record_desc_t *record_desc)
{
    ret_code_t ret;
    
    // Flash ROMから既存データを走査
    bool found = false;
    fds_find_token_t  ftok = {0};
    do {
        ret = fds_record_find(FIDO_SKEY_CERT_FILE_ID, FIDO_TOKEN_COUNTER_RECORD_KEY, record_desc, &ftok);
        if (ret == FDS_SUCCESS) {
            // 同じappIdHashのレコードかどうか判定 (先頭32バイトを比較)
            token_counter_record_get(record_desc, m_token_counter_record_buffer);
            if (strncmp((char *)p_appid_hash, (char *)m_token_counter_record_buffer, 32) == 0) {
                found = true;
            }
        }
    } while (ret == FDS_SUCCESS && found == false);

    return found;
}

uint32_t fido_flash_token_counter_value(void)
{
    // カウンターを取得して戻す
    return m_token_counter_record_buffer[8];
}

bool fido_flash_token_counter_read(uint8_t *p_appid_hash)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_token_counter_record_bufferに読込む
    fds_record_desc_t record_desc;
    return token_counter_record_find(p_appid_hash, &record_desc);
}

bool fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint32_t reserve_word)
{
    // Flash ROMから既存データを走査
    bool found = false;
    fds_record_desc_t record_desc;
    found = token_counter_record_find(p_appid_hash, &record_desc);
    
    // ユニークキーとなるappIdHash部 (8ワード)
    memcpy((uint8_t *)m_token_counter_record_buffer, p_appid_hash, 32);

    // トークンカウンター部 (1ワード)
    m_token_counter = token_counter;
    m_token_counter_record_buffer[8] = m_token_counter;

    // 予備部 (1ワード)
    m_reserve_word = reserve_word;
    m_token_counter_record_buffer[9] = m_reserve_word;

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = FIDO_SKEY_CERT_FILE_ID;
    record.key               = FIDO_TOKEN_COUNTER_RECORD_KEY;
    record.data.p_data       = m_token_counter_record_buffer;
    record.data.length_words = 10;

    ret_code_t ret;
    if (found == true) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fido_flash_token_counter_write: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fido_flash_token_counter_write: fds_record_write returns 0x%02x ", ret);
            return false;
        }
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        NRF_LOG_ERROR("fido_flash_token_counter_write: no space in flash, calling FDS GC ");
        if (fido_flash_force_fdc_gc() == false) {
            return false;
        }
    }

    return true;
}
