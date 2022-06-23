/* 
 * File:   ccid_flash_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/06/15, 15:09
 */
//
// プラットフォーム非依存コード
//
#include <stdlib.h>
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"
#include "fido_flash_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_flash_oath_object);
#endif

#define LOG_SETTINGS_KEY_FOR_FIND       false

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

static bool get_record_key_by_tag(uint16_t obj_tag, uint16_t *file_id, uint16_t *record_key)
{   
    // ファイルID
    *file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードID
    switch (obj_tag) {
        case OATH_TAG_NAME:
            *record_key = OATH_DATA_OBJ_01_RECORD_KEY;
            break;
        default:
            return false;
    }
    return true;
}

bool ccid_flash_oath_object_write(uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size, bool use_serial, uint16_t serial)
{
    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &file_id, &record_key) == false) {
        return false;
    }

    // 引数のデータを、Flash ROM書込み用データの一時格納領域にコピー
    //   オブジェクトデータの長さ: 1ワード（4バイト）
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint8_t *rec_bytes = ccid_flash_object_write_buffer();
    uint32_t size32_t = (uint32_t)obj_size;
    memcpy(rec_bytes, &size32_t, sizeof(uint32_t));
    memcpy(rec_bytes + 4, obj_buff, obj_size);

    // オブジェクトデータの長さから、必要ワード数を計算
    size_t record_words = ccid_flash_object_calculate_words(size32_t) + 1;

    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {file_id, record_key, use_serial, serial};
    size_t size = record_words * sizeof(uint32_t);

    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_oath_object_write;

    if (app_settings_save(&key, (void *)rec_bytes, size)) {
        // 書込み成功の場合は、OpenPGP関連処理を継続
        app_event_notify(APEVT_APP_SETTINGS_SAVED);
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

//
// データ検索
//
// 同一キー判定用
static void  *m_unique_key;
static size_t m_unique_key_size;

// レコード内のユニークキー開始位置
static size_t m_unique_key_offset;

// レコード連番を保持
static uint16_t m_serial;
static uint16_t m_serial_max;

// 作業領域
static char work_buf[16];

static bool get_serial_from_settings_key(const char *settings_key, uint16_t *serial)
{
    // settings_key が14バイトでない場合は終了
    if (strlen(settings_key) != 14) {
        *serial = 0;
        return false;
    }

    // settings_keyから連番を取り出す
    // 先頭の11バイト目から4バイト
    //   settings_key: "BFFB/BFEB/nnnn"形式。先頭の"app/"は含まれない
    strncpy(work_buf, settings_key + 10, 4);

    // 連番を引数領域に設定
    int i = atoi(work_buf);
    *serial = (uint16_t)i;
    return true;
}

static bool compare_unique_key(const char *settings_key, void *data, size_t size)
{
    // settings_keyから連番を取り出す
    uint16_t serial;
    (void)get_serial_from_settings_key(settings_key, &serial);

    // 最大の連番を更新
    if (serial > m_serial_max) {
        m_serial_max = serial;
    }

    // 検索レコード内のユニークキー開始位置を設定
    uint8_t *p_unique_key = (uint8_t *)data + m_unique_key_offset;
    (void)size;

#if LOG_SETTINGS_KEY_FOR_FIND
    LOG_INF("settings key=%s", log_strdup(settings_key));
    LOG_HEXDUMP_INF(p_unique_key, 16, "unique key in data");
    LOG_HEXDUMP_INF(m_unique_key, 16, "unique key for search");
#endif

    // 同じキーのレコードかどうか判定 (先頭から比較)
    if (memcmp(p_unique_key, m_unique_key, m_unique_key_size) == 0) {
        // 同一キーレコードの連番を保持
        m_serial = serial;
        return true;

    } else {
        m_serial = 0;
        return false;
    }
}

bool ccid_flash_oath_object_find(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // 最大連番をゼロクリア
    m_serial_max = 0;

    // レコード検索用ユニークキーの参照／サイズを保持
    m_unique_key = (void *)p_unique_key;
    m_unique_key_size = unique_key_size;

    // 検索レコード内のユニークキー開始位置を保持
    // （先頭４バイトは、オブジェクトデータの長さが格納されている）
    m_unique_key_offset = 4;

    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &file_id, &record_key) == false) {
        return false;
    }

    // Flash ROMから既存データを走査
    APP_SETTINGS_KEY key = {file_id, record_key, false, 0};
    size_t size;
    if (app_settings_search(&key, exist, p_record_buffer, &size, compare_unique_key) == false) {
        return false;
    }

    // レコードの連番
    if (*exist) {
        // 既存のデータが存在する場合
        *serial = m_serial;

    } else {
        // 既存のデータが存在しない場合
        *serial = m_serial_max + 1;
    }

    // 既存データがあれば true
    return true;
}

bool ccid_flash_oath_object_delete(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &file_id, &record_key) == false) {
        return false;
    }

    // 削除対象レコードを検索
    if (ccid_flash_oath_object_find(obj_tag, p_unique_key, unique_key_size, p_record_buffer, exist, serial) == false) {
        return false;
    }

    // 削除対象レコードがない場合はここで終了
    if (*exist == false) {
        return true;
    } 
    
    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_oath_object_delete;

    // 該当レコードをFlash ROM領域から削除
    APP_SETTINGS_KEY key = {file_id, record_key, true, *serial};
    if (app_settings_delete_multi(&key)) {
        // 削除成功の場合は、管理用コマンドの処理を継続
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
void ccid_flash_oath_object_record_updated(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_oath_object_write) {
        ccid_oath_object_write_resume(true);
    }
}

void ccid_flash_oath_object_record_deleted(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_oath_object_delete) {
        ccid_oath_object_write_resume(true);
    }
}
