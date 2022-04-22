/* 
 * File:   ccid_flash_piv_object.c
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#include <stdbool.h>
#include <stddef.h>
//
// プラットフォーム非依存コード
//
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"
#include "ccid_piv_object_import.h"
#include "ccid_ykpiv.h"
#include "fido_flash_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
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

static bool read_piv_object_data_from_app_settings(uint8_t obj_tag, bool *is_exist)
{
    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // 確保領域は0で初期化
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    memset(read_buffer, 0, sizeof(uint32_t) * ccid_flash_object_rw_buffer_size());

    // Flash ROMから既存データを検索し、
    // 見つかった場合は read_buffer にデータをセット
    //   見つからなかった場合、read_buffer にデータはセットされないが、
    //   関数戻り値は true とします。
    APP_SETTINGS_KEY key = {PIV_DATA_OBJ_FILE_ID, record_key, false, 0};
    size_t size;
    if (app_settings_find(&key, is_exist, (void *)read_buffer, &size) == false) {
        return false;
    }

    return true;
}

static bool write_piv_object_data_to_app_settings(uint8_t obj_tag, uint8_t obj_alg, uint8_t *obj_data, size_t obj_data_size)
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

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }

    // オブジェクトデータの長さから、必要ワード数を計算
    size_t record_words = PIV_DATA_OBJ_ATTR_WORDS + ccid_flash_object_calculate_words(size32_t);
    
    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {PIV_DATA_OBJ_FILE_ID, record_key, false, 0};
    size_t size = record_words * sizeof(uint32_t);

    if (app_settings_save(&key, (void *)rec_bytes, size)) {
        // 書込み成功の場合は、PIV関連処理を継続
        app_event_notify(APEVT_APP_SETTINGS_SAVED);
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
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
    if (read_piv_object_data_from_app_settings(key_tag, is_exist) == false) {
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
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_piv_object_card_admin_key_write;

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    uint8_t key_tag = TAG_KEY_CAADM;
    return write_piv_object_data_to_app_settings(key_tag, key_alg, key, key_size);
}

//
// PIV秘密鍵関連
//
bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t *key_size, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (read_piv_object_data_from_app_settings(key_tag, is_exist) == false) {
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
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_piv_object_private_key_write;

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    return write_piv_object_data_to_app_settings(key_tag, key_alg, key, key_size);
}

//
// PIV PIN／リトライカウンター関連
//
bool ccid_flash_piv_object_pin_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_piv_object_pin_write;

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    uint8_t obj_alg = 0xff;
    return write_piv_object_data_to_app_settings(obj_tag, obj_alg, obj_data, obj_size);
}

//
// PIVデータオブジェクト関連
//
bool ccid_flash_piv_object_data_read(uint8_t obj_tag, uint8_t *obj_data, size_t *obj_size, bool *is_exist)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データをバッファに読込む
    if (read_piv_object_data_from_app_settings(obj_tag, is_exist) == false) {
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
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_piv_object_data_write;

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピーし、
    // Flash ROMに書込
    uint8_t obj_alg = 0xff;
    return write_piv_object_data_to_app_settings(obj_tag, obj_alg, obj_data, obj_size);
}

bool ccid_flash_piv_object_data_erase(void)
{
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_piv_object_data_erase;

    // 全てのPIVオブジェクトデータをFlash ROM領域から削除
    APP_SETTINGS_KEY key = {PIV_DATA_OBJ_FILE_ID, 0, false, 0};
    if (app_settings_delete_multi(&key)) {
        // 削除成功の場合は、PIV関連処理を継続
        app_event_notify(APEVT_APP_SETTINGS_DELETED);
        return true;

    } else {
        // 削除失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

//
// コールバック関数群
//
void ccid_flash_piv_object_failed(void)
{
}

void ccid_flash_piv_object_gc_done(void)
{
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

void ccid_flash_piv_object_record_deleted(void)
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
