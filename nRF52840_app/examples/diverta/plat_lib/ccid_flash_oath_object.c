/* 
 * File:   ccid_flash_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/11/15, 17:44
 */
#include "sdk_common.h"

#include "fido_flash_plat.h"
#include "fido_flash_common.h"

#include "ccid_flash_object.h"
#include "ccid_oath.h"
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ccid_flash_oath_object
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// バッファサイズの最大長
#define MAX_BUF_SIZE     (OATH_DATA_OBJ_WORDS_MAX+1)

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
//
static bool get_record_key_by_tag(uint8_t tag, uint16_t *record_key)
{
    switch (tag) {
        default:
            *record_key = OATH_DATA_OBJ_01_RECORD_KEY;
            break;
    }
    return true;
}

bool ccid_flash_oath_object_write(uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size, bool use_serial, uint16_t serial)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_find(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // レコードキーを取得
    uint16_t record_key;
    if (get_record_key_by_tag(obj_tag, &record_key) == false) {
        return false;
    }
    
    // Flash ROMから属性データ（オブジェクト長＝１ワード）を読込
    uint32_t *read_buffer = (uint32_t *)ccid_flash_object_read_buffer();
    if (fido_flash_fds_record_read(file_id, record_key, OATH_DATA_OBJ_ATTR_WORDS, read_buffer, exist) == false) {
        return false;
    }
    // 既存データがなければ、最初の連番をセットして終了
    if (*exist == false) {
        *serial = 1;
        return true;
    }

    // TODO: 仮の実装です。
    return true;
}

bool ccid_flash_oath_object_delete(uint16_t obj_tag, uint8_t *p_unique_key, size_t unique_key_size, uint8_t *p_record_buffer, bool *exist, uint16_t *serial)
{
    // TODO: 仮の実装です。
    return false;
}

bool ccid_flash_oath_object_delete_all(void)
{
    // ファイル名を取得
    uint16_t file_id = OATH_DATA_OBJ_FILE_ID;

    // データファイルをFlash ROMから削除
    m_flash_func = (void *)ccid_flash_oath_object_delete_all;
    return fido_flash_fds_file_delete(file_id);
}

bool ccid_flash_oath_object_fetch(uint16_t obj_tag, int (*_fetch_func)(const char *key, void *data, size_t size))
{
    // TODO: 仮の実装です。
    return false;
}

//
// コールバック関数群
//
void ccid_flash_oath_object_failed(void)
{
    if (m_flash_func == NULL) {
        return;
    }

    // 判定用の参照を初期化
    void *flash_func = m_flash_func;
    m_flash_func = NULL;

    // Flash ROM処理でエラーが発生時はエラーレスポンス送信
    if (flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_resume(false);
    }
}

void ccid_flash_oath_object_gc_done(void)
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
    if (flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_retry();
    }
}

void ccid_flash_oath_object_record_updated(void)
{
    // TODO: 仮の実装です。
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
    if (flash_func == ccid_flash_oath_object_delete_all) {
        ccid_oath_object_write_resume(true);
    }
}
