#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>

#include "ble_u2f_keypair.h"
#include "ble_u2f_util.h"
#include "fds.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_flash"
#include "nrf_log.h"


// Flash ROMに保存するための
// ファイルID、レコードKey
//  0xBFFE: 鍵・証明書
//  0xBFFD: トークンカウンター
#define U2F_FILE_ID            (0xBFFE)
#define U2F_KEYPAIR_RECORD_KEY (0xBFFE)
#define U2F_TOKEN_COUNTER_RECORD_KEY (0xBFFD)

// Flash ROM書込み用データの一時格納領域
static fds_record_chunk_t  m_fds_record_chunks[3];
static uint32_t m_token_counter_record_buffer[10];
static uint32_t m_token_counter;
static uint32_t m_reserve_word;


bool ble_u2f_flash_keydata_delete(void)
{
    ret_code_t err_code;

    // 秘密鍵／証明書をFlash ROM領域から削除
    NRF_LOG_DEBUG("ble_u2f_keypare_erase start \r\n");
    err_code = fds_file_delete(U2F_FILE_ID);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x \r\n", err_code);
        return false;
    }

    // ガベージコレクションを実行する
    err_code = fds_gc();
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_gc returns 0x%02x \r\n", err_code);
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
        NRF_LOG_ERROR("fds_record_open returns 0x%02x \r\n", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->tl.length_words;
    memcpy(keydata_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x \r\n", err_code);
        return false;	
    }
    return true;
}


static uint32_t *keydata_buffer_allocate(ble_u2f_context_t *p_u2f_context)
{
    uint32_t *keydata_buffer = p_u2f_context->keypair_cert_buffer;
    uint16_t  keypair_buffer_length = p_u2f_context->keypair_cert_buffer_length;
    if (keydata_buffer != NULL) {
        // 既に確保済みの場合
        NRF_LOG_DEBUG("keypair_cert_buffer already allocated (%d bytes) \r\n", 
            keypair_buffer_length);
        return keydata_buffer;
    }

    // Flash ROM読込用領域の確保
    keypair_buffer_length = sizeof(uint32_t) * KEYPAIR_CERT_WORD_NUM;
    keydata_buffer = (uint32_t *)malloc(keypair_buffer_length);
    if (keydata_buffer == NULL) {
        // 領域が確保出来なかったら終了
        NRF_LOG_ERROR("keypair_cert_buffer allocation failed \r\n");
        return keydata_buffer;
    }
    NRF_LOG_DEBUG("keypair_cert_buffer allocated (%d bytes) \r\n", keypair_buffer_length);

    // 確保した領域のアドレスと長さを共有情報に保持
    p_u2f_context->keypair_cert_buffer = keydata_buffer;
    p_u2f_context->keypair_cert_buffer_length = keypair_buffer_length;

    // 確保領域は0xff（Flash ROM未書込状態）で初期化
    memset(keydata_buffer, 0xff, keypair_buffer_length);
    return keydata_buffer;
}

bool ble_u2f_flash_keydata_read(ble_u2f_context_t *p_u2f_context)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    uint32_t *keydata_buffer = keydata_buffer_allocate(p_u2f_context);
    if (keydata_buffer == NULL) {
        return false;
    }

    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(U2F_FILE_ID, U2F_KEYPAIR_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        if (keydata_record_get(&record_desc, keydata_buffer) == false) {
            return false;
        }

    } else if (ret != FDS_ERR_NOT_FOUND) {
        // レコードが存在しないときはOK
        // それ以外の場合はエラー終了
        NRF_LOG_ERROR("fds_record_find returns 0x%02x \r\n", ret);
        return false;
    }
    
    return true;
}


bool ble_u2f_flash_keydata_write(ble_u2f_context_t *p_u2f_context)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    uint32_t *keydata_buffer = p_u2f_context->keypair_cert_buffer;
    m_fds_record_chunks[0].p_data       = keydata_buffer;
    m_fds_record_chunks[0].length_words = KEYPAIR_CERT_WORD_NUM;

    fds_record_t record;
    record.file_id         = U2F_FILE_ID;
    record.key             = U2F_KEYPAIR_RECORD_KEY;
    record.data.p_chunks   = m_fds_record_chunks;
    record.data.num_chunks = 1;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(U2F_FILE_ID, U2F_KEYPAIR_RECORD_KEY, &record_desc, &ftok);
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
            // 書込みができない場合はエラー扱い
            // (Disconnect時にGCを実行させて終了）
            NRF_LOG_ERROR("ble_u2f_flash_keydata_write: no space in flash. call GC later \r\n");
            p_u2f_context->need_fdc_gc = true;
            return false;

        } else if (ret != FDS_SUCCESS) {
            NRF_LOG_ERROR("ble_u2f_flash_keydata_write: fds returns 0x%02x \r\n", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("fds_record_find returns 0x%02x \r\n", ret);
        return false;
    }
    
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
        NRF_LOG_ERROR("fds_record_open returns 0x%02x \r\n", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->tl.length_words;
    memcpy(token_counter_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("fds_record_close returns 0x%02x \r\n", err_code);
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
        ret = fds_record_find(U2F_FILE_ID, U2F_TOKEN_COUNTER_RECORD_KEY, record_desc, &ftok);
        if (ret == FDS_SUCCESS) {
            // 同じappIdHashのレコードかどうか判定 (先頭32バイトを比較)
            token_counter_record_get(record_desc, m_token_counter_record_buffer);
            if (strncmp((char *)p_appid_hash, (char *)m_token_counter_record_buffer, 32) == 0) {
                found = true;
            }
        }
    } while (ret == FDS_SUCCESS);

    return found;
}

uint32_t ble_u2f_flash_token_counter_value(void)
{
    // カウンターを取得して戻す
    return m_token_counter_record_buffer[8];
}

bool ble_u2f_flash_token_counter_read(uint8_t *p_appid_hash)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_token_counter_record_bufferに読込む
    fds_record_desc_t record_desc;
    return token_counter_record_find(p_appid_hash, &record_desc);
}

bool ble_u2f_flash_token_counter_write(ble_u2f_context_t *p_u2f_context, uint8_t *p_appid_hash, uint32_t token_counter, uint32_t reserve_word)
{
    // Flash ROMから既存データを走査
    bool found = false;
    fds_record_desc_t record_desc;
    found = token_counter_record_find(p_appid_hash, &record_desc);
    
    // ユニークキーとなるappIdHash部 (8ワード)
    m_fds_record_chunks[0].p_data       = p_appid_hash;
    m_fds_record_chunks[0].length_words = 8;

    // トークンカウンター部 (1ワード)
    m_token_counter = token_counter;
    m_fds_record_chunks[1].p_data       = &m_token_counter;
    m_fds_record_chunks[1].length_words = 1;

    // 予備部 (1ワード)
    m_reserve_word = reserve_word;
    m_fds_record_chunks[2].p_data       = &m_reserve_word;
    m_fds_record_chunks[2].length_words = 1;

    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id         = U2F_FILE_ID;
    record.key             = U2F_TOKEN_COUNTER_RECORD_KEY;
    record.data.p_chunks   = m_fds_record_chunks;
    record.data.num_chunks = 3;

    ret_code_t ret;
    if (found == true) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &record);

    } else {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &record);
    }
        
    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合はエラー扱い
        // (Disconnect時にGCを実行させて終了）
        NRF_LOG_ERROR("ble_u2f_flash_token_counter_write: no space in flash. call GC later \r\n");
        p_u2f_context->need_fdc_gc = true;
        return false;

    } else if (ret != FDS_SUCCESS) {
        NRF_LOG_ERROR("ble_u2f_flash_token_counter_write: fds returns 0x%02x \r\n", ret);
        return false;
    }
    
    return true;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
