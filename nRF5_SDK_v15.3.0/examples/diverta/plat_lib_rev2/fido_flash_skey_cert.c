/* 
 * File:   fido_flash_skey_cert.c
 * Author: makmorit
 *
 * Created on 2019/07/09, 10:21
 */
#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fido_flash.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_flash_skey_cert
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み用データの一時格納領域
static fds_record_t m_fds_record;

// 鍵・証明書データ読込用の作業領域（固定長）
static uint32_t skey_cert_data[SKEY_CERT_WORD_NUM];

uint32_t *fido_flash_skey_cert_data(void)
{
    return skey_cert_data;
}

uint8_t *fido_flash_skey_data(void)
{
    // 秘密鍵格納領域の開始アドレスを取得
    uint32_t *skey_buffer = fido_flash_skey_cert_data();
    return (uint8_t *)skey_buffer;
}

uint8_t *fido_flash_cert_data(void)
{
    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = fido_flash_skey_cert_data() + SKEY_WORD_NUM + 1;
    return (uint8_t *)cert_buffer;
}

uint32_t fido_flash_cert_data_length(void)
{
    // 証明書データ格納領域の長さを取得
    uint32_t *cert_buffer = fido_flash_skey_cert_data() + SKEY_WORD_NUM;
    uint32_t cert_buffer_length = *cert_buffer;
    return cert_buffer_length;
}

bool fido_flash_skey_cert_delete(void)
{
    // 秘密鍵／証明書をFlash ROM領域から削除
    ret_code_t err_code = fds_file_delete(FIDO_SKEY_CERT_FILE_ID);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x ", err_code);
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
        if (fido_flash_fds_record_get(&record_desc, skey_cert_data) == false) {
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

bool fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length)
{
    // 秘密鍵部（リクエストデータの先頭32バイト）を領域に格納
    uint32_t *securekey_buffer = fido_flash_skey_cert_data();
    memcpy(securekey_buffer, data, 32);

    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = securekey_buffer + SKEY_WORD_NUM;
    uint16_t  cert_length = length - 32;

    // 証明書データの格納に必要なワード数をチェックする
    uint32_t cert_buffer_length = (cert_length - 1) / 4 + 2;
    if (cert_buffer_length > CERT_WORD_NUM) {
        NRF_LOG_ERROR("cert data words(%d) exceeds max words(%d) ",
            cert_buffer_length, CERT_WORD_NUM);
        return false;
    }
    
    // １ワード目に、証明書の当初バイト数を格納し、
    // ２ワード目以降から、証明書のデータを格納するようにする
    // (エンディアンは変換せずにそのまま格納)
    cert_buffer[0] = (uint32_t)cert_length;
    memcpy(cert_buffer + 1, data + 32, cert_length);
    return true;
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
            NRF_LOG_ERROR("fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("fds_record_find returns 0x%02x ", ret);
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
