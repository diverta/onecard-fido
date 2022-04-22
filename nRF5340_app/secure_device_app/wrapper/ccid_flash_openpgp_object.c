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

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
//
bool ccid_flash_openpgp_object_read(CCID_APPLET applet_id, uint16_t obj_tag, bool *is_exist, uint8_t *buff, size_t *size)
{
    return false;
}

bool ccid_flash_openpgp_object_write(CCID_APPLET applet_id, uint16_t obj_tag, uint8_t *obj_buff, size_t obj_size)
{
    return false;
}

bool ccid_flash_openpgp_object_delete_all(CCID_APPLET applet_id)
{
    return false;
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
