/* 
 * File:   ccid_flash_object.c
 * Author: makmorit
 *
 * Created on 2021/02/15, 11:58
 */
#include "sdk_common.h"

#include "ccid_flash_object.h"
#include "ccid_openpgp.h"
#include "ccid_openpgp_object.h"
#include "fido_flash.h"
#include "fido_flash_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// Flash ROM書込み時に実行中のAPPLETを保持
static CCID_APPLET m_applet_id = APPLET_NONE;

// レコード格納領域
//   バッファ長（MAX_BUF_SIZE）は、
//   このモジュールで管理する
//   最大のレコードサイズに合わせます。
#define MAX_BUF_SIZE     (OPGP_DATA_OBJ_WORDS_MAX+1)
static uint32_t          m_record_buf_R[MAX_BUF_SIZE];
static uint32_t          m_record_buf_W[MAX_BUF_SIZE];

uint8_t *ccid_flash_object_read_buffer(void)
{
    return (uint8_t *)m_record_buf_R;
}

uint8_t *ccid_flash_object_write_buffer(void)
{
    return (uint8_t *)m_record_buf_W;
}

size_t ccid_flash_object_rw_buffer_size(void)
{
    return MAX_BUF_SIZE;
}

//
// オブジェクトのRead／Write
//
static bool get_file_id(CCID_APPLET applet_id, uint16_t *file_id)
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

static bool get_record_key_by_tag(CCID_APPLET applet_id, uint16_t obj_tag, uint16_t *file_id, uint16_t *record_key)
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
        default:
            return false;
    }
    return true;
}

bool ccid_flash_object_read_by_tag(CCID_APPLET applet_id, uint16_t obj_tag, bool *is_exist, uint8_t *obj_buff, size_t *obj_size)
{
    // 引数からファイル名、レコードキーを取得
    uint16_t file_id;
    uint16_t record_key;
    if (get_record_key_by_tag(applet_id, obj_tag, &file_id, &record_key) == false) {
        return false;
    }

    // Flash ROMからデータを読込
    //   オブジェクトデータの長さ: 1ワード（4バイト）
    //   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    if (fido_flash_fds_record_read(file_id, record_key, MAX_BUF_SIZE, read_buffer, is_exist) == false) {
        return false;
    }
 
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

static size_t calculate_record_words(size_t record_bytes)
{
    // オブジェクトの長さから、必要ワード数を計算
    size_t record_words = record_bytes / 4;
    if (record_bytes % 4 > 0) {
        record_words++;
    }
    return record_words;
}

bool ccid_flash_object_write_by_tag(CCID_APPLET applet_id, uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size)
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
    size_t record_words = calculate_record_words(size32_t) + 1;

    // 実行中のAPPLETを保持
    m_applet_id = applet_id;
    
    // データをFlash ROMに書込
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    uint32_t *write_buffer = (uint32_t *)ccid_flash_object_write_buffer();
    return fido_flash_fds_record_write(file_id, record_key, record_words, read_buffer, write_buffer);
}

bool ccid_flash_object_delete_all(CCID_APPLET applet_id)
{
    // 引数からファイル名を取得
    uint16_t file_id;
    if (get_file_id(applet_id, &file_id) == false) {
        return false;
    }

    // 実行中のAPPLETを保持
    m_applet_id = applet_id;

    // データファイルをFlash ROMから削除
    return fido_flash_fds_file_delete(file_id);
}

//
// コールバック関数群
//
void ccid_flash_object_failed(void)
{
    if (m_applet_id == APPLET_NONE) {
        return;
    }

    // 判定用の参照を初期化
    CCID_APPLET applet_id = m_applet_id;
    m_applet_id = APPLET_NONE;

    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    if (applet_id == APPLET_OPENPGP) {
        ccid_openpgp_object_write_resume(false);
    }
}

void ccid_flash_object_gc_done(void)
{
    if (m_applet_id == APPLET_NONE) {
        return;
    }

    // 判定用の参照を初期化
    CCID_APPLET applet_id = m_applet_id;
    m_applet_id = APPLET_NONE;

    // for nRF52840:
    // FDSリソース不足解消のためGCが実行された場合は、
    // GC実行直前の処理を再実行
    if (applet_id == APPLET_OPENPGP) {
        ccid_openpgp_object_write_retry();
    }
}

void ccid_flash_object_record_updated(void)
{
    if (m_applet_id == APPLET_NONE) {
        return;
    }

    // 判定用の参照を初期化
    CCID_APPLET applet_id = m_applet_id;
    m_applet_id = APPLET_NONE;

    // 正常系の後続処理を実行
    if (applet_id == APPLET_OPENPGP) {
        ccid_openpgp_object_write_resume(true);
    }
}

void ccid_flash_object_record_deleted(void)
{
    if (m_applet_id == APPLET_NONE) {
        return;
    }

    // 判定用の参照を初期化
    CCID_APPLET applet_id = m_applet_id;
    m_applet_id = APPLET_NONE;

    // 正常系の後続処理を実行
    if (applet_id == APPLET_OPENPGP) {
        ccid_openpgp_object_write_resume(true);
    }
}
