/* 
 * File:   ccid_flash_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 17:23
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "fido_flash_common.h"

#include "ccid_flash_object.h"
#include "ccid_ykpiv.h"
#include "ccid_piv_define.h"
#include "ccid_piv_object.h"
#include "ccid_piv_object_import.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_piv_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug data
#define LOG_HEXDUMP_DEBUG false

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// Flash ROM read/write
//
static bool get_record_key_by_tag(uint8_t tag, uint16_t *record_key)
{
    switch (tag) {
        case TAG_OBJ_CHUID:
            *record_key = PIV_DATA_OBJ_02_RECORD_KEY;
            break;
        case TAG_CERT_PAUTH:
            *record_key = PIV_DATA_OBJ_05_RECORD_KEY;
            break;
        case TAG_OBJ_CCC:
            *record_key = PIV_DATA_OBJ_07_RECORD_KEY;
            break;
        case TAG_CERT_DGSIG:
            *record_key = PIV_DATA_OBJ_0A_RECORD_KEY;
            break;
        case TAG_CERT_KEYMN:
            *record_key = PIV_DATA_OBJ_0B_RECORD_KEY;
            break;
        case TAG_KEY_PAUTH:
            *record_key = PIV_DATA_OBJ_9A_RECORD_KEY;
            break;
        case TAG_KEY_CAADM:
            *record_key = PIV_DATA_OBJ_9B_RECORD_KEY;
            break;
        case TAG_KEY_DGSIG:
            *record_key = PIV_DATA_OBJ_9C_RECORD_KEY;
            break;
        case TAG_KEY_KEYMN:
            *record_key = PIV_DATA_OBJ_9D_RECORD_KEY;
            break;
        case TAG_PIV_PIN:
            *record_key = PIV_DATA_OBJ_80_RECORD_KEY;
            break;
        case TAG_KEY_PUK:
            *record_key = PIV_DATA_OBJ_81_RECORD_KEY;
            break;
        default:
            return false;
    }
    return true;
}

//
// PIVオブジェクトのRead／Write
//
static bool read_piv_object_data_from_fds(uint8_t obj_tag, bool *is_exist)
{
    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // Flash ROMから属性データを読込
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    if (fido_flash_fds_record_read(PIV_DATA_OBJ_FILE_ID, record_key, PIV_DATA_OBJ_ATTR_WORDS, read_buffer, is_exist) == false) {
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
    uint8_t *rec_bytes = ccid_flash_object_read_buffer();
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
    size_t record_words = PIV_DATA_OBJ_ATTR_WORDS + ccid_flash_object_calculate_words(size32_t);

    // Flash ROMからオブジェクトデータを読込
    //   データが存在する場合は、
    //   m_record_buf_Rの３ワード目を先頭とし、
    //   オブジェクトデータが格納されます
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    return fido_flash_fds_record_read(PIV_DATA_OBJ_FILE_ID, record_key, record_words, read_buffer, is_exist);
}

static bool write_piv_object_data_to_fds(uint8_t obj_tag, uint8_t obj_alg, uint8_t *obj_data, size_t obj_data_size)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    //   オブジェクト属性 = 2ワード
    //     属性データ: 1ワード（4バイト）
    //       0    : 種別（1バイト）
    //       1    : アルゴリズム（1バイト）
    //       2 - 3: 予備（2バイト）
    //     オブジェクトデータの長さ: 1ワード（4バイト）
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint8_t *rec_bytes = ccid_flash_object_write_buffer();
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
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key)) {
        // オブジェクトデータの長さから、必要ワード数を計算し、
        // データをFlash ROMに書込
        size_t record_words = PIV_DATA_OBJ_ATTR_WORDS + ccid_flash_object_calculate_words(size32_t);
        uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
        uint32_t *write_buffer = (uint32_t *)ccid_flash_object_write_buffer();
        return fido_flash_fds_record_write(PIV_DATA_OBJ_FILE_ID, record_key, record_words, read_buffer, write_buffer);

    } else {
        return false;
    }
}

static void copy_object_data_from_buffer(uint8_t *obj_alg, uint8_t *obj_data_buf, size_t *obj_data_size)
{
    // オブジェクトの属性を取得
    uint8_t *rec_bytes = ccid_flash_object_read_buffer();
    uint32_t size32_t;
    *obj_alg = rec_bytes[1];
    memcpy(&size32_t, rec_bytes + 4, sizeof(uint32_t));

    // データを引数の領域にコピー
    memcpy(obj_data_buf, rec_bytes + 8, size32_t);
    *obj_data_size = size32_t;
}

//
// 管理用パスワード関連
//
bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    uint8_t key_tag = TAG_KEY_CAADM;
    if (read_piv_object_data_from_fds(key_tag, is_exist) == false) {
        return false;
    }
    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
    }
    // データを引数の領域にコピー
    copy_object_data_from_buffer(key_alg, key, key_size);
    return true;
}

bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg)
{
    uint8_t key_tag = TAG_KEY_CAADM;

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    m_flash_func = (void *)ccid_flash_piv_object_card_admin_key_write;
    return write_piv_object_data_to_fds(key_tag, key_alg, key, key_size);
}

//
// PIV秘密鍵関連
//
bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t *key_size, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (read_piv_object_data_from_fds(key_tag, is_exist) == false) {
        return false;
    }

    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
    }

    // データを引数の領域にコピー
    uint8_t key_alg_;
    copy_object_data_from_buffer(&key_alg_, key, key_size);

    // アルゴリズムが登録されているものと異なる場合は、
    // 既存データ無しと扱う
    if (key_alg != key_alg_) {
        *is_exist = false;
    }

    return true;
}

bool ccid_flash_piv_object_private_key_write(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t key_size)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    m_flash_func = (void *)ccid_flash_piv_object_private_key_write;
    return write_piv_object_data_to_fds(key_tag, key_alg, key, key_size);
}

//
// PIV PIN／リトライカウンター関連
//
bool ccid_flash_piv_object_pin_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    uint8_t obj_alg = 0xff;
    m_flash_func = (void *)ccid_flash_piv_object_pin_write;
    return write_piv_object_data_to_fds(obj_tag, obj_alg, obj_data, obj_size);
}

//
// PIVデータオブジェクト関連
//
bool ccid_flash_piv_object_data_read(uint8_t obj_tag, uint8_t *obj_data, size_t *obj_size, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (read_piv_object_data_from_fds(obj_tag, is_exist) == false) {
        return false;
    }

    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
    }

    // データを引数の領域にコピー
    uint8_t obj_alg;
    copy_object_data_from_buffer(&obj_alg, obj_data, obj_size);
    return true;
}

bool ccid_flash_piv_object_data_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    uint8_t obj_alg = 0xff;
    m_flash_func = (void *)ccid_flash_piv_object_data_write;
    return write_piv_object_data_to_fds(obj_tag, obj_alg, obj_data, obj_size);
}

bool ccid_flash_piv_object_data_erase(void)
{
    // 全てのPIVオブジェクトデータをFlash ROM領域から削除
    ret_code_t err_code = fds_file_delete(PIV_DATA_OBJ_FILE_ID);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("fds_file_delete returns 0x%02x ", err_code);
        return false;
    }

    m_flash_func = (void *)ccid_flash_piv_object_data_erase;
    return true;
}

void ccid_flash_piv_object_failed(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    if (flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_resume(false);
    }
    if (flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_resume(false);
    }
    if (flash_func == (void *)ccid_flash_piv_object_data_write) {
        ccid_piv_object_import_resume(false);
    }
    if (flash_func == (void *)ccid_flash_piv_object_pin_write) {
        ccid_piv_object_pin_set_resume(false);
    }
    if (flash_func == (void *)ccid_flash_piv_object_data_erase) {
        ccid_ykpiv_ins_reset_resume(false);
    }
}

void ccid_flash_piv_object_gc_done(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    if (flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_retry();
    }
    if (flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_retry();
    }
    if (flash_func == (void *)ccid_flash_piv_object_data_write) {
        ccid_piv_object_import_retry();
    }
    if (flash_func == (void *)ccid_flash_piv_object_pin_write) {
        ccid_piv_object_pin_set_retry();
    }
    if (flash_func == (void *)ccid_flash_piv_object_data_erase) {
        ccid_ykpiv_ins_reset_retry();
    }
}

void ccid_flash_piv_object_record_updated(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == (void *)ccid_flash_piv_object_card_admin_key_write) {
        ccid_ykpiv_ins_set_mgmkey_resume(true);
    }
    if (flash_func == (void *)ccid_flash_piv_object_private_key_write) {
        ccid_ykpiv_ins_import_key_resume(true);
    }
    if (flash_func == (void *)ccid_flash_piv_object_data_write) {
        ccid_piv_object_import_resume(true);
    }
    if (flash_func == (void *)ccid_flash_piv_object_pin_write) {
        ccid_piv_object_pin_set_resume(true);
    }
}

void ccid_flash_piv_object_file_deleted(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == (void *)ccid_flash_piv_object_data_erase) {
        ccid_ykpiv_ins_reset_resume(true);
    }
}
