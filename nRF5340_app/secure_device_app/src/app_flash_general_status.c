/* 
 * File:   app_flash_general_status.c
 * Author: makmorit
 *
 * Created on 2023/04/25, 13:51
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// プラットフォーム依存コード
#include "app_flash_general_status.h"
#include "app_settings.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(app_flash_general_status);
#endif

//
//  汎用ステータス関連
//
// Flash ROM書込み用データの一時格納領域
static uint32_t m_general_status;

// Flash ROMに保存するための
// ファイルID、レコードKey
//
// File IDs should be in the range 0x0000 - 0xBFFF.
// Record keys should be in the range 0x0001 - 0xBFFF.
// https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.0.0/lib_fds_functionality.html
//
// nRF52840で「ペアリングモードレコード」として割り当てられているIDを再利用します。
//
#define FIDO_PAIRING_MODE_FILE_ID     (0xBFFD)
#define FIDO_PAIRING_MODE_RECORD_KEY  (0xBFED)

//
// 内部処理
//
static bool delete_general_status(void)
{
    // Flash ROM領域から削除
    APP_SETTINGS_KEY key = {FIDO_PAIRING_MODE_FILE_ID, 0, false, 0};
    if (app_settings_delete_multi(&key) == false) {
        return false;
    }
    return true;
}

static bool read_general_status(bool *p_exist)
{
    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    APP_SETTINGS_KEY key = {FIDO_PAIRING_MODE_FILE_ID, FIDO_PAIRING_MODE_RECORD_KEY, false, 0};
    size_t size = 0;
    if (app_settings_find(&key, p_exist, (void *)&m_general_status, &size) == false) {
        return false;
    }
    return true;
}

static bool write_general_status(void)
{
    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {FIDO_PAIRING_MODE_FILE_ID, FIDO_PAIRING_MODE_RECORD_KEY, false, 0};
    size_t size = sizeof(uint32_t);
    if (app_settings_save(&key, (void *)&m_general_status, size) == false) {
        return false;
    }
    return true;
}

//
// 公開用関数
//
bool app_flash_general_status_flag(void)
{
    bool exist;
    if (read_general_status(&exist) && exist) {
        return app_flash_general_status_flag_get();

    } else {
        return false;
    }
}

bool app_flash_general_status_flag_get(void)
{
    return (m_general_status == 1);
}

void app_flash_general_status_flag_clear(void)
{
    m_general_status = 0;
    write_general_status();
}

void app_flash_general_status_flag_set(void)
{
    m_general_status = 1;
    write_general_status();
}

void app_flash_general_status_flag_reset(void)
{
    m_general_status = 0;
    delete_general_status();
}

//
// テスト用
//
void app_flash_general_status_test(void)
{
    bool mode;

    app_flash_general_status_flag_reset();
    mode = app_flash_general_status_flag();
    printk("app_flash_general_status_flag_reset done (mode=%d) \n", mode);

    app_flash_general_status_flag_clear();
    mode = app_flash_general_status_flag();
    printk("app_flash_general_status_flag_clear done (mode=%d) \n", mode);

    app_flash_general_status_flag_set();
    mode = app_flash_general_status_flag();
    printk("app_flash_general_status_flag_set done (mode=%d) \n", mode);

    app_flash_general_status_flag_clear();
    mode = app_flash_general_status_flag();
    printk("app_flash_general_status_flag_clear done (mode=%d) \n", mode);
}
