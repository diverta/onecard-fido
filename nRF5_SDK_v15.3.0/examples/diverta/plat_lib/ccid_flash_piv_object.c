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

// for debug data
#define LOG_HEXDUMP_DEBUG false

// レコード格納領域
//   バッファ長（MAX_BUF_SIZE）は、
//   このモジュールで管理する
//   最大のレコードサイズに合わせます。
#define MAX_BUF_SIZE     128
static uint32_t          m_record_buf_R[MAX_BUF_SIZE];
static uint32_t          m_record_buf_W[MAX_BUF_SIZE];


bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (fido_flash_fds_record_read(PIV_DATA_OBJ_9D_FILE_ID, PIV_DATA_OBJ_9D_RECORD_KEY, PIV_DATA_OBJ_9D_RECORD_SIZE, m_record_buf_R) == false) {
        return false;
    }

    // Flash ROM書込み用データの一時格納領域
    //   レコードサイズ = 9 ワード
    //     パスワード: 8ワード（32バイト)
    //     属性データ: 1ワード（4バイト）
    //       0    : パスワード長（1バイト）
    //       1    : パスワードのアルゴリズム（1バイト）
    //       2 - 3: 予備（2バイト）
    uint8_t *rec_bytes = (uint8_t *)m_record_buf_R;
    uint8_t size = rec_bytes[32];
    uint8_t alg  = rec_bytes[33];

    if (size == 0xff || alg == 0xff) {
        // データが不正な場合は終了
        return false;
    }

#if LOG_HEXDUMP_DEBUG
    NRF_LOG_DEBUG("Management key from Flash ROM (%d bytes) alg=0x%02x", size, alg);
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes, size);
#endif

    // データを引数の領域にコピー
    *key_size = size;
    *key_alg  = alg;
    memcpy(key, rec_bytes, size);
    return true;
}

bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    uint8_t *rec_bytes = (uint8_t *)m_record_buf_W;
    rec_bytes[32] = (uint8_t)key_size;
    rec_bytes[33] = key_alg;
    memcpy(rec_bytes, key, key_size);

#if LOG_HEXDUMP_DEBUG
    NRF_LOG_DEBUG("Management key to Flash ROM (%d bytes) alg=0x%02x", key_size, key_alg);
    NRF_LOG_HEXDUMP_DEBUG(key, key_size);
#endif

    // データをFlash ROMに書込
    return fido_flash_fds_record_write(PIV_DATA_OBJ_9D_FILE_ID, PIV_DATA_OBJ_9D_RECORD_KEY, PIV_DATA_OBJ_9D_RECORD_SIZE, m_record_buf_R, m_record_buf_W);
}
