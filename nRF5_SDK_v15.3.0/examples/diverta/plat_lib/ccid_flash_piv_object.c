/* 
 * File:   ccid_flash_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 17:23
 */
#include "sdk_common.h"

#include "fido_flash.h"
#include "fido_flash_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_piv_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "ccid_flash_piv_object.h"

// Flash ROM書込み用データの一時格納領域
// 
// 管理用パスワード
//   レコードサイズ = 9 ワード
//     パスワード: 8ワード（32バイト)
//     属性データ: 1ワード（4バイト）
//       0    : パスワード長（1バイト）
//       1    : パスワードのアルゴリズム（1バイト）
//       2 - 3: 予備（2バイト）
static uint32_t m_card_admin_key_record[PIV_DATA_OBJ_9D_RECORD_SIZE];

// レコード格納領域
static fds_record_desc_t record_desc;

static bool piv_object_record_find(uint16_t file_id, uint16_t record_key, size_t record_words, uint32_t *record_buff)
{
    // 作業領域の初期化
    memset(record_buff, 0, record_words * 4);

    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(file_id, record_key, &record_desc, &ftok);
    if (ret != FDS_SUCCESS) {
        return false;
    }

    // Flash ROMに登録されているデータを読み出す
    return fido_flash_fds_record_get(&record_desc, record_buff);
}

static bool piv_object_record_write(uint16_t file_id, uint16_t record_key, size_t record_words, uint32_t *record_buff)
{
    // Flash ROMから既存データを走査
    bool found = piv_object_record_find(file_id, record_key, record_words, record_buff);
    
    // Flash ROMに書込むレコードを生成
    fds_record_t record;
    record.file_id           = file_id;
    record.key               = record_key;
    record.data.p_data       = record_buff;
    record.data.length_words = record_words;

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

bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (piv_object_record_find(PIV_DATA_OBJ_9D_FILE_ID, PIV_DATA_OBJ_9D_RECORD_KEY, PIV_DATA_OBJ_9D_RECORD_SIZE, m_card_admin_key_record) == false) {
        return false;
    }

    // Flash ROM書込み用データの一時格納領域
    //   レコードサイズ = 9 ワード
    //     パスワード: 8ワード（32バイト)
    //     属性データ: 1ワード（4バイト）
    //       0    : パスワード長（1バイト）
    //       1    : パスワードのアルゴリズム（1バイト）
    //       2 - 3: 予備（2バイト）
    uint8_t *rec_bytes = (uint8_t *)m_card_admin_key_record;
    uint8_t size = rec_bytes[32];
    uint8_t alg  = rec_bytes[33];

    if (size == 0xff || alg == 0xff) {
        // データが不正な場合は終了
        return false;
    }

    // データを引数の領域にコピー
    *key_size = size;
    *key_alg  = alg;
    memcpy(key, rec_bytes, size);
    return true;
}

bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    uint8_t *rec_bytes = (uint8_t *)m_card_admin_key_record;
    rec_bytes[32] = (uint8_t)key_size;
    rec_bytes[33] = key_alg;
    memcpy(rec_bytes, key, key_size);

    // データをFlash ROMに書込
    if (piv_object_record_write(PIV_DATA_OBJ_9D_FILE_ID, PIV_DATA_OBJ_9D_RECORD_KEY, PIV_DATA_OBJ_9D_RECORD_SIZE, m_card_admin_key_record) == false) {
        return false;
    }

    return true;
}
