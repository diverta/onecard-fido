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

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// オブジェクトのRead／Write
//
bool ccid_flash_piv_object_card_admin_key_read(uint8_t *key, size_t *key_size, uint8_t *key_alg, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_card_admin_key_write(uint8_t *key, size_t key_size, uint8_t key_alg)
{
    return false;
}

bool ccid_flash_piv_object_private_key_read(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t *key_size, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_private_key_write(uint8_t key_tag, uint8_t key_alg, uint8_t *key, size_t key_size)
{
    return false;
}

bool ccid_flash_piv_object_pin_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    return false;
}

bool ccid_flash_piv_object_data_read(uint8_t obj_tag, uint8_t *obj_data, size_t *obj_size, bool *is_exist)
{
    return false;
}

bool ccid_flash_piv_object_data_write(uint8_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    return false;
}

bool ccid_flash_piv_object_data_erase(void)
{
    return false;
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
