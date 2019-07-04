/* 
 * File:   fido_flash_password.c
 * Author: makmorit
 *
 * Created on 2018/12/27, 14:46
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include "fds.h"
#include "fido_crypto.h"
#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_password
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Flash ROM書込み用データの一時格納領域
static fds_record_t m_fds_record;
static uint32_t     m_random_vector[8];

static bool write_random_vector(uint32_t *p_fds_record_buffer)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    m_fds_record.data.p_data       = p_fds_record_buffer;
    m_fds_record.data.length_words = 8;
    m_fds_record.file_id           = FIDO_AESKEYS_FILE_ID;
    m_fds_record.key               = FIDO_AESKEYS_RECORD_KEY;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(FIDO_AESKEYS_FILE_ID, FIDO_AESKEYS_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("write_random_vector: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("write_random_vector: fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("write_random_vector: fds_record_find returns 0x%02x ", ret);
        return false;
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        NRF_LOG_ERROR("write_random_vector: no space in flash, calling FDS GC ");
        if (fido_flash_force_fdc_gc() == false) {
            return false;
        }
    }

    return true;
}

bool fido_flash_password_generate(void)
{
    // 32バイトのランダムベクターを生成
    fido_crypto_generate_random_vector((uint8_t *)m_random_vector, 32);

    // Flash ROMに書き出して保存
    if (write_random_vector(m_random_vector) == false) {
        return false;
    }

    NRF_LOG_DEBUG("Generated random vector for AES password ");
    return true;
}


static bool read_random_vector_record(fds_record_desc_t *record_desc, uint32_t *data_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("read_random_vector_record: fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(data_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("read_random_vector_record: fds_record_close returns 0x%02x ", err_code);
        return false;	
    }
    return true;
}

static bool read_random_vector(uint32_t *p_fds_record_buffer)
{
    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(FIDO_AESKEYS_FILE_ID, FIDO_AESKEYS_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        if (read_random_vector_record(&record_desc, p_fds_record_buffer) == false) {
            // データ格納失敗時
            return false;
        }
    } else {
        // レコードが存在しないときや
        // その他エラー発生時
        return false;
    }
    return true;
}

uint8_t *fido_flash_password_get(void)
{
    if (read_random_vector(m_random_vector) == false) {
        // Flash ROMにランダムベクターを格納したレコードが存在しない場合
        // 処理終了
        return NULL;
    }

    // Flash ROMレコードから取り出したランダムベクターを
    // パスワードに設定
    return (uint8_t *)m_random_vector;
}
