/* 
 * File:   ccid_flash_openpgp_object.c
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#include <stdbool.h>
#include <stddef.h>
//
// プラットフォーム非依存コード
//
#include "ccid_openpgp.h"
#include "ccid_openpgp_object.h"
#include "fido_flash_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// バッファサイズの最大長
#define MAX_BUF_SIZE     (OPGP_DATA_OBJ_WORDS_MAX+1)

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
//
static bool get_file_id(uint8_t applet_id, uint16_t *file_id)
{
    switch (applet_id) {
        case APPLET_OPENPGP:
            *file_id = OPGP_DATA_OBJ_FILE_ID;
            break;
        default:
            return false;
    }
    return true;
}

static bool get_record_key_by_tag(uint8_t applet_id, uint16_t obj_tag, uint16_t *file_id, uint16_t *record_key)
{
    if (get_file_id(applet_id, file_id) == false) {
        return false;
    }
    switch (obj_tag) {
        case TAG_OPGP_PW1:
            *record_key = OPGP_DATA_OBJ_01_RECORD_KEY;
            break;
        case TAG_OPGP_PW3:
            *record_key = OPGP_DATA_OBJ_02_RECORD_KEY;
            break;
        case TAG_OPGP_RC:
            *record_key = OPGP_DATA_OBJ_03_RECORD_KEY;
            break;
        case TAG_ATTR_TERMINATED:
            *record_key = OPGP_DATA_OBJ_04_RECORD_KEY;
            break;
        case TAG_PW_STATUS:
            *record_key = OPGP_DATA_OBJ_05_RECORD_KEY;
            break;
        case TAG_ALGORITHM_ATTRIBUTES_SIG:
            *record_key = OPGP_DATA_OBJ_06_RECORD_KEY;
            break;
        case TAG_ALGORITHM_ATTRIBUTES_DEC:
            *record_key = OPGP_DATA_OBJ_07_RECORD_KEY;
            break;
        case TAG_ALGORITHM_ATTRIBUTES_AUT:
            *record_key = OPGP_DATA_OBJ_08_RECORD_KEY;
            break;
        case TAG_KEY_SIG_STATUS:
            *record_key = OPGP_DATA_OBJ_09_RECORD_KEY;
            break;
        case TAG_KEY_DEC_STATUS:
            *record_key = OPGP_DATA_OBJ_10_RECORD_KEY;
            break;
        case TAG_KEY_AUT_STATUS:
            *record_key = OPGP_DATA_OBJ_11_RECORD_KEY;
            break;
        case TAG_KEY_SIG:
            *record_key = OPGP_DATA_OBJ_12_RECORD_KEY;
            break;
        case TAG_KEY_DEC:
            *record_key = OPGP_DATA_OBJ_13_RECORD_KEY;
            break;
        case TAG_KEY_AUT:
            *record_key = OPGP_DATA_OBJ_14_RECORD_KEY;
            break;
        case TAG_KEY_SIG_FINGERPRINT:
            *record_key = OPGP_DATA_OBJ_15_RECORD_KEY;
            break;
        case TAG_KEY_DEC_FINGERPRINT:
            *record_key = OPGP_DATA_OBJ_16_RECORD_KEY;
            break;
        case TAG_KEY_AUT_FINGERPRINT:
            *record_key = OPGP_DATA_OBJ_17_RECORD_KEY;
            break;
        case TAG_KEY_SIG_GENERATION_DATES:
            *record_key = OPGP_DATA_OBJ_18_RECORD_KEY;
            break;
        case TAG_KEY_DEC_GENERATION_DATES:
            *record_key = OPGP_DATA_OBJ_19_RECORD_KEY;
            break;
        case TAG_KEY_AUT_GENERATION_DATES:
            *record_key = OPGP_DATA_OBJ_20_RECORD_KEY;
            break;
        case TAG_DIGITAL_SIG_COUNTER:
            *record_key = OPGP_DATA_OBJ_21_RECORD_KEY;
            break;
        case TAG_KEY_CA1_FINGERPRINT:
        case TAG_KEY_CA2_FINGERPRINT:
        case TAG_KEY_CA3_FINGERPRINT:
            *record_key = 0xBFFF;
            break;
        default:
            return false;
    }
    return true;
}

bool ccid_flash_openpgp_object_read(CCID_APPLET applet_id, uint16_t obj_tag, bool *is_exist, uint8_t *obj_buff, size_t *obj_size)
{
    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(applet_id, obj_tag, &file_id, &record_key) == false) {
        return false;
    }

    // 確保領域は0で初期化
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    memset(read_buffer, 0, sizeof(uint32_t) * ccid_flash_object_rw_buffer_size());

    // Flash ROMから既存データを検索し、
    // 見つかった場合は read_buffer にデータをセット
    //   見つからなかった場合、read_buffer にデータはセットされないが、
    //   関数戻り値は true とします。
    APP_SETTINGS_KEY key = {file_id, record_key, false, 0};
    size_t size;
    if (app_settings_find(&key, is_exist, (void *)read_buffer, &size) == false) {
        return false;
    }

#if LOG_HEXDUMP_OBJECT_DATA
    LOG_HEXDUMP_INF((void *)read_buffer, size, "Object data");
#endif

    // 既存データがなければここで終了
    if (*is_exist == false) {
        return true;
    }

    // オブジェクトデータの長さを取得
    size_t total_size = read_buffer[0];

    // 読み込んだデータを、引数の領域にコピー
    if (obj_size != NULL) {
        *obj_size = total_size;
    }
    if (obj_buff != NULL) {
        memcpy(obj_buff, ccid_flash_object_read_buffer() + 4, total_size);
    }
    return true;
}

bool ccid_flash_openpgp_object_write(CCID_APPLET applet_id, uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size)
{
    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(applet_id, obj_tag, &file_id, &record_key) == false) {
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
    APP_SETTINGS_KEY key = {file_id, record_key, false, 0};
    size_t size = record_words * sizeof(uint32_t);

    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_openpgp_object_write;

    if (app_settings_save(&key, (void *)rec_bytes, size)) {
        // 書込み成功の場合は、OpenPGP関連処理を継続
        app_event_notify(APEVT_APP_SETTINGS_SAVED);
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

bool ccid_flash_openpgp_object_delete_all(CCID_APPLET applet_id)
{
    // 引数からファイル名を取得
    uint16_t file_id;
    if (get_file_id(applet_id, &file_id) == false) {
        return false;
    }

    // Flash ROM更新関数の参照を保持
    m_flash_func = (void *)ccid_flash_openpgp_object_delete_all;

    // 全てのOpenPGPオブジェクトデータをFlash ROM領域から削除
    APP_SETTINGS_KEY key = {file_id, 0, false, 0};
    if (app_settings_delete_multi(&key)) {
        // 削除成功の場合は、OpenPGP関連処理を継続
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
void ccid_flash_openpgp_object_failed(void)
{
}

void ccid_flash_openpgp_object_gc_done(void)
{
}

void ccid_flash_openpgp_object_record_updated(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_openpgp_object_write || 
        flash_func == ccid_flash_openpgp_object_delete_all) {
        ccid_openpgp_object_write_resume(true);
    }
}

void ccid_flash_openpgp_object_record_deleted(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // 正常系の後続処理を実行
    if (flash_func == ccid_flash_openpgp_object_write || 
        flash_func == ccid_flash_openpgp_object_delete_all) {
        ccid_openpgp_object_write_resume(true);
    }
}
