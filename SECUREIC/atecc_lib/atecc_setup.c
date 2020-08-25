/* 
 * File:   atecc_setup.c
 * Author: makmorit
 *
 * Created on 2020/08/12, 11:45
 */
//
// プラットフォーム非依存コード
//
#include "atecc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "atecc_command.h"
#include "atecc_iface.h"
#include "atecc_read.h"
#include "atecc_util.h"
#include "atecc_write.h"

static uint8_t ecc608_configdata[ATECC_CONFIG_SIZE] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xEE, 0x01, 0x4D, 0x00,
    0xC0, 0x00, 0x55, 0x00,
    // SlotConfig
    0x83, 0x20, // slot 0
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x00, 0x00, 
    0x87, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20, 
    0x83, 0x20,
    0x83, 0x6F, 
    0x8F, 0x0F, // slot 15
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x8F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
    0x55,       // LockValue
    0x55,       // LockConfig
    0xFF, 0xFF, // SlotLocked
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    // KeyConfig
    0x33, 0x00, // slot 0
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x3C, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00,
    0x33, 0x00, 
    0x3C, 0x00  // slot 15
};

static bool get_zone_is_locked(bool *is_config_locked, bool *is_data_locked)
{
    if (atecc_lock_status_get(LOCK_ZONE_CONFIG, is_config_locked) == false) {
        fido_log_error("atecc_setup_config: atecc_lock_status_get(LOCK_ZONE_CONFIG) failed");
        return false;
    }
    if (atecc_lock_status_get(LOCK_ZONE_DATA, is_data_locked) == false) {
        fido_log_error("atecc_setup_config: atecc_lock_status_get(LOCK_ZONE_DATA) failed");
        return false;
    }
    return true;
}

static bool lock_config_zone(void)
{
    bool status = atecc_lock_config_zone();
    if (status == false) {
        fido_log_error("lock_config_zone failed: atecc_lock_config_zone returns false");
        return false;
    }

    fido_log_info("lock_config_zone success");
    return true;
}

static bool lock_data_zone(void)
{
    bool status = atecc_lock_data_zone();
    if (status == false) {
        fido_log_error("lock_data_zone failed: atecc_lock_data_zone returns false");
        return false;
    }

    fido_log_info("lock_data_zone success");
    return true;
}

static bool write_config_zone(void)
{
    // Configバイトを書き込み
    bool status = atecc_write_config_zone(ecc608_configdata);
    if (status == false) {
        fido_log_error("write_config_zone failed: atecc_write_config_zone returns false");
        return false;
    }

    fido_log_info("write_config_zone success");
    return true;
}

bool atecc_setup_config(void)
{
    // Config情報を取得
    if (atecc_get_config_bytes() == false) {
        return false;
    }

    // ロック状況を取得
    bool is_config_locked;
    bool is_data_locked;
    if (get_zone_is_locked(&is_config_locked, &is_data_locked) == false) {
        return false;
    }
    if (is_config_locked && is_data_locked) {
        // Config、Data共にロックされている場合は終了
        fido_log_info("atecc_setup_config: Config and data zone is already locked");
        return true;
    } 

    if (is_config_locked == false) {
        // Configがロックされていない場合は、
        // Config変更とロックを実行
        if (write_config_zone() == false) {
            return false;
        }
        if (lock_config_zone() == false) {
            return false;
        }        
    } 

    if (is_data_locked == false) {
        // Dataがロックされていない場合はロックを実行
        if (lock_data_zone() == false) {
            return false;
        }        
    }

    // Config情報を再取得
    if (atecc_get_config_bytes() == false) {
        return false;
    }

    fido_log_info("atecc_setup_config done");
    return true;
}
