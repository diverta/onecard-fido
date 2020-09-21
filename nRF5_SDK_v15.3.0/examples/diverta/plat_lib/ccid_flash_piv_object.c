/* 
 * File:   ccid_flash_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 17:23
 */
#include "sdk_common.h"

#include "fido_flash.h"
#include "fido_flash_common.h"

#include "ccid_ykpiv.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"

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
#define MAX_BUF_SIZE     PIV_DATA_OBJ_DATA_WORDS_MAX
static uint32_t          m_record_buf_R[MAX_BUF_SIZE];
static uint32_t          m_record_buf_W[MAX_BUF_SIZE];

uint8_t *ccid_flash_piv_object_read_buffer(void)
{
    return (uint8_t *)m_record_buf_R;
}

uint8_t *ccid_flash_piv_object_write_buffer(void)
{
    return (uint8_t *)m_record_buf_W;
}

size_t ccid_flash_piv_object_rw_buffer_size(void)
{
    return MAX_BUF_SIZE;
}

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// Flash ROM read/write
//
static size_t get_private_key_length_words(uint8_t key_alg)
{
    size_t length_words;
    switch (key_alg) {
        case ALG_RSA_2048:
            length_words = RSA2048_KEY_SIZE / 4;
            break;
        case ALG_ECC_256:
            length_words = ECC_PRV_KEY_SIZE / 4;
            break;
        default:
            length_words = 0;
            break;
    }
    return length_words;
}

static bool get_record_key_by_tag(uint8_t tag, uint8_t alg, uint16_t *file_id, uint16_t *record_key, size_t *record_words)
{
    switch (tag) {
        case TAG_OBJ_CHUID:
            *file_id = PIV_DATA_OBJ_CERT_FILE_ID;
            *record_key = PIV_DATA_OBJ_02_RECORD_KEY;
            *record_words = MAX_CHUID_SIZE / 4;
            break;
        case TAG_CERT_PAUTH:
            *file_id = PIV_DATA_OBJ_CERT_FILE_ID;
            *record_key = PIV_DATA_OBJ_05_RECORD_KEY;
            *record_words = MAX_CERT_SIZE / 4;
            break;
        case TAG_OBJ_CCC:
            *file_id = PIV_DATA_OBJ_CERT_FILE_ID;
            *record_key = PIV_DATA_OBJ_07_RECORD_KEY;
            *record_words = MAX_CCC_SIZE / 4;
            break;
        case TAG_CERT_DGSIG:
            *file_id = PIV_DATA_OBJ_CERT_FILE_ID;
            *record_key = PIV_DATA_OBJ_0A_RECORD_KEY;
            *record_words = MAX_CERT_SIZE / 4;
            break;
        case TAG_CERT_KEYMN:
            *file_id = PIV_DATA_OBJ_CERT_FILE_ID;
            *record_key = PIV_DATA_OBJ_0B_RECORD_KEY;
            *record_words = MAX_CERT_SIZE / 4;
            break;
        case TAG_KEY_PAUTH:
            *file_id = PIV_DATA_OBJ_PRVKEY_FILE_ID;
            *record_key = PIV_DATA_OBJ_9A_RECORD_KEY;
            *record_words = get_private_key_length_words(alg);
            break;
        case TAG_KEY_DGSIG:
            *file_id = PIV_DATA_OBJ_PRVKEY_FILE_ID;
            *record_key = PIV_DATA_OBJ_9C_RECORD_KEY;
            *record_words = get_private_key_length_words(alg);
            break;
        case TAG_KEY_KEYMN:
            *file_id = PIV_DATA_OBJ_PRVKEY_FILE_ID;
            *record_key = PIV_DATA_OBJ_9D_RECORD_KEY;
            *record_words = get_private_key_length_words(alg);
            break;
        default:
            return false;
    }
    // レコード長は、属性データの１ワード分を加算
    *record_words += 1;
    return true;
}

static bool read_piv_object_data(uint8_t obj_tag, uint8_t obj_alg, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    uint16_t file_id;
    uint16_t record_key;
    size_t record_words;
    if (get_record_key_by_tag(obj_tag, obj_alg, &file_id, &record_key, &record_words)) {
        return fido_flash_fds_record_read(file_id, record_key, record_words, m_record_buf_R, is_exist);
    } else {
        return false;
    }
}

static bool write_piv_object_data(uint8_t obj_tag, uint8_t obj_alg, size_t object_size)
{
    // 属性データをバッファに設定
    //   0    : 種別（1バイト）
    //   1    : アルゴリズム（1バイト）
    //   2 - 3: データの長さ（2バイト）
    uint8_t *rec_bytes = ccid_flash_piv_object_write_buffer();
    rec_bytes[0] = obj_tag;
    rec_bytes[1] = obj_alg;
    uint16_t size_16t = (uint16_t)object_size;
    memcpy(rec_bytes + 2, &size_16t, sizeof(uint16_t));

    // データをFlash ROMに書込
    uint16_t file_id;
    uint16_t record_key;
    size_t record_words;
    if (get_record_key_by_tag(obj_tag, obj_alg, &file_id, &record_key, &record_words)) {
        return fido_flash_fds_record_write(file_id, record_key, record_words, m_record_buf_R, m_record_buf_W);
    } else {
        return false;
    }
}

//
// PIVオブジェクトのRead／Write
//
static size_t calculate_record_words(size_t record_bytes)
{
    // オブジェクトの長さから、必要ワード数を計算
    size_t record_words = record_bytes / 4;
    if (record_bytes % 4 > 0) {
        record_words++;
    }
    return record_words;
}

static bool read_piv_object_data_from_fds(uint8_t obj_tag, uint8_t obj_alg, bool *is_exist)
{
    // レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    size_t record_words;
    if (get_record_key_by_tag(obj_tag, obj_alg, &file_id, &record_key, &record_words) == false) {
        return false;
    }

    // Flash ROMから属性データを読込
    if (fido_flash_fds_record_read(PIV_DATA_OBJ_FILE_ID, record_key, PIV_DATA_OBJ_ATTR_WORDS, m_record_buf_R, is_exist) == false) {
        return false;
    }
    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
    }

    // 属性データを取出し、一時変数に保持
    //   オブジェクト属性 = 2ワード
    //     属性データ: 1ワード（4バイト）
    //       0    : 種別（1バイト）
    //       1    : アルゴリズム（1バイト）
    //       2 - 3: 予備（2バイト）
    //     オブジェクトデータの長さ: 1ワード（4バイト）
    uint8_t *rec_bytes = ccid_flash_piv_object_read_buffer();
    uint32_t size32_t;
    memcpy(&size32_t, rec_bytes + 4, sizeof(uint32_t));

#if LOG_HEXDUMP_DEBUG
    size_t total_size = size32_t + 8;
    NRF_LOG_DEBUG("ccid_flash_piv_object_read_buffer (%d bytes)", total_size);
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes, 32);
    NRF_LOG_DEBUG("last 16 bytes:");
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes + total_size - 16, 16);
#endif

    // オブジェクトデータの長さから、必要ワード数を計算
    record_words = PIV_DATA_OBJ_ATTR_WORDS + calculate_record_words(size32_t);

    // Flash ROMからオブジェクトデータを読込
    //   データが存在する場合は、
    //   m_record_buf_Rの３ワード目を先頭とし、
    //   オブジェクトデータが格納されます
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    return fido_flash_fds_record_read(PIV_DATA_OBJ_FILE_ID, record_key, record_words, m_record_buf_R, is_exist);
}

static bool write_piv_object_data_to_fds(uint8_t obj_tag, uint8_t obj_alg, uint8_t *obj_data, size_t obj_data_size, bool *is_exist)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    //   オブジェクト属性 = 2ワード
    //     属性データ: 1ワード（4バイト）
    //       0    : 種別（1バイト）
    //       1    : アルゴリズム（1バイト）
    //       2 - 3: 予備（2バイト）
    //     オブジェクトデータの長さ: 1ワード（4バイト）
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint8_t *rec_bytes = ccid_flash_piv_object_write_buffer();
    rec_bytes[0] = obj_tag;
    rec_bytes[1] = obj_alg;
    uint32_t size32_t = (uint32_t)obj_data_size;
    memcpy(rec_bytes + 4, &size32_t, sizeof(uint32_t));
    memcpy(rec_bytes + 8, obj_data, obj_data_size);

#if LOG_HEXDUMP_DEBUG
    size_t total_size = obj_data_size + 8;
    NRF_LOG_DEBUG("ccid_flash_piv_object_write_buffer (%d bytes)", total_size);
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes, 32);
    NRF_LOG_DEBUG("last 16 bytes:");
    NRF_LOG_HEXDUMP_DEBUG(rec_bytes + total_size - 16, 16);
#endif

    // レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    size_t record_words;
    if (get_record_key_by_tag(obj_tag, obj_alg, &file_id, &record_key, &record_words)) {
        // オブジェクトデータの長さから、必要ワード数を計算し、
        // データをFlash ROMに書込
        record_words = PIV_DATA_OBJ_ATTR_WORDS + calculate_record_words(size32_t);
        return fido_flash_fds_record_write(PIV_DATA_OBJ_FILE_ID, record_key, record_words, m_record_buf_R, m_record_buf_W);

    } else {
        return false;
    }
}

//
// 管理用パスワード関連
//
bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (fido_flash_fds_record_read(PIV_DATA_OBJ_9B_FILE_ID, PIV_DATA_OBJ_9B_RECORD_KEY, PIV_DATA_OBJ_9B_RECORD_SIZE, m_record_buf_R, is_exist) == false) {
        return false;
    }
    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
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
    m_flash_func = (void *)ccid_flash_piv_object_card_admin_key_write;
    return fido_flash_fds_record_write(PIV_DATA_OBJ_9B_FILE_ID, PIV_DATA_OBJ_9B_RECORD_KEY, PIV_DATA_OBJ_9B_RECORD_SIZE, m_record_buf_R, m_record_buf_W);
}

//
// PIV秘密鍵関連
//
bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    return read_piv_object_data(key_tag, key_alg, is_exist);
}

bool ccid_flash_piv_object_private_key_write(uint8_t key_tag, uint8_t key_alg)
{
    // 鍵データ長を設定
    size_t key_size = ECC_PRV_KEY_SIZE;
    if (key_alg == ALG_RSA_2048) {
        key_size = RSA2048_KEY_SIZE;
    }

    // 呼び出し元の関数名を保持
    m_flash_func = (void *)ccid_flash_piv_object_private_key_write;

    // データをFlash ROMに書込
    return write_piv_object_data(key_tag, key_alg, key_size);
}

void ccid_flash_piv_object_failed(void)
{
    if (m_flash_func == NULL) {
        return;
    }
    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    if (m_flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_resume(false);
    }
    if (m_flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_resume(false);
    }
    m_flash_func = NULL;
}

void ccid_flash_piv_object_gc_done(void)
{
    if (m_flash_func == NULL) {
        return;
    }
    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    if (m_flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_retry();
    }
    if (m_flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_retry();
    }
    m_flash_func = NULL;
}

void ccid_flash_piv_object_record_updated(void)
{
    if (m_flash_func == NULL) {
        return;
    }
    // 正常系の後続処理を実行
    if (m_flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_resume(true);
    }
    if (m_flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_resume(true);
    }
    m_flash_func = NULL;
}
