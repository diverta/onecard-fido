/* 
 * File:   fido_cryptoauth_setup.c
 * Author: makmorit
 *
 * Created on 2019/12/16, 15:18
 */
//
// プラットフォーム非依存コード
//
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_setup.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "atca_execution.h"
#include "cryptoauthlib.h"

static uint8_t ecc608_configdata[ATCA_ECC_CONFIG_SIZE] = {
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
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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

static bool atcau_is_locked(uint8_t zone, bool *is_locked)
{
    ATCA_STATUS status = ATCA_GEN_FAIL;
    ATCAPacket packet;
    ATCACommand ca_cmd = _gDevice->mCommands;

    // build an read command
    packet.param1 = 0x00;
    packet.param2 = 0x15;
    status = atRead(ca_cmd, &packet);
    if (status != ATCA_SUCCESS) {
        return false;
    }

    status = atca_execute_command(&packet, _gDevice);
    if (status != ATCA_SUCCESS) {
        return false;
    }

    switch (zone) {
    case LOCK_ZONE_DATA:
        *is_locked = (packet.data[ATCA_RSP_DATA_IDX + 2] == 0);
        break;
    case LOCK_ZONE_CONFIG:
        *is_locked = (packet.data[ATCA_RSP_DATA_IDX + 3] == 0);
        break;
    default:
        return false;
    }

    return true;
}

static bool lock_config_zone(void)
{
    ATCA_STATUS status = atcab_lock_config_zone();
    if (status != ATCA_SUCCESS) {
        fido_log_error("lock_config_zone: atcab_lock_config_zone failed with ret=0x%02x", status);
        return false;
    }

    fido_log_info("lock_config_zone success");
    return true;
}

static bool lock_data_zone(void)
{
    ATCA_STATUS status = atcab_lock_data_zone();
    if (status != ATCA_SUCCESS) {
        fido_log_error("lock_data_zone: atcab_lock_data_zone failed with ret=0x%02x", status);
        return false;
    }

    fido_log_info("lock_data_zone success");
    return true;
}

static bool write_config_zone(void)
{
    // Configバイトを書き込み
    ATCA_STATUS status = atcab_write_config_zone(ecc608_configdata);
    if (status != ATCA_SUCCESS) {
        fido_log_error("write_config_zone: atcab_write_config_zone failed with ret=0x%02x", status);
        return false;
    }

    fido_log_info("write_config_zone success");
    return true;
}

bool fido_cryptoauth_setup_config(void)
{
    // Config情報を取得
    if (fido_cryptoauth_get_config_bytes() == false) {
        return false;
    }

    // ロック状況を取得
    bool is_config_locked;
    bool is_data_locked;
    if (atcau_is_locked(LOCK_ZONE_CONFIG, &is_config_locked) == false) {
        fido_log_error("fido_cryptoauth_setup_config: atcau_is_locked(LOCK_ZONE_CONFIG) failed");
        return false;
    }
    if (atcau_is_locked(LOCK_ZONE_DATA, &is_data_locked) == false) {
        fido_log_error("fido_cryptoauth_setup_config: atcau_is_locked(LOCK_ZONE_DATA) failed");
        return false;
    }
    if (is_config_locked && is_data_locked) {
        // Config、Data共にロックされている場合は終了
        fido_log_info("fido_cryptoauth_setup_config: Config and data zone is already locked");
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
    if (fido_cryptoauth_get_config_bytes() == false) {
        return false;
    }

    fido_log_info("fido_cryptoauth_setup_config done");
    return true;
}
