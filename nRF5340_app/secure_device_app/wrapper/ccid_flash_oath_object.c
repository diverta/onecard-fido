/* 
 * File:   ccid_flash_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/06/15, 15:09
 */
//
// プラットフォーム非依存コード
//
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"
#include "fido_flash_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

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
