/* 
 * File:   fido_cryptoauth.c
 * Author: makmorit
 *
 * Created on 2019/11/25, 14:43
 */
// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "atca_iface.h"
#include "cryptoauthlib.h"

// 設定情報を保持
static ATCAIfaceCfg m_iface_config;
static uint8_t ateccx08a_config_bytes[ATCA_ECC_CONFIG_SIZE];

// 初期化処理実行済みフラグ
static bool atcab_init_done = false;

// シリアルナンバーを保持
static uint8_t cryptoauth_serial_num[ATCA_SERIAL_NUM_SIZE];

// 公開鍵を保持
static uint8_t public_key_raw_data[ATCA_PUB_KEY_SIZE];

static bool init_device(ATCAIfaceCfg *p_cfg)
{
    if (atcab_init_done) {
        // 初期化処理が実行済みの場合は終了
        fido_log_debug("select_device already done");
        return true;
    }

    // デバイス設定は、ライブラリーのデフォルトを採用
    *p_cfg = cfg_ateccx08a_i2c_default;

    // デバイスの初期化
    ATCA_STATUS status = atcab_init(p_cfg);
    if (status != ATCA_SUCCESS) {
        fido_log_error("select_device failed: atcab_init() returns 0x%08x", status);
        return false;
    }

    // 初期化処理は実行済み
    atcab_init_done = true;
    return true;
}

static bool get_cryptoauth_serial_num(void)
{
    ATCA_STATUS status = atcab_read_serial_number(cryptoauth_serial_num);
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_serial_no: atcab_read_serial_number() failed with ret=0x%08x", status);
        return false;
    }
    fido_log_debug("Serial number:");
    fido_log_print_hexdump_debug(cryptoauth_serial_num, ATCA_SERIAL_NUM_SIZE);
    return true;
}

static bool get_cryptoauth_config_bytes(void)
{
    ATCA_STATUS status = atcab_read_config_zone(ateccx08a_config_bytes);
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_cryptoauth_config_bytes: atcab_read_config_zone() failed with ret=0x%08x", status);
        return false;
    }
    fido_log_debug("Config zone data (128 bytes):");
    fido_log_print_hexdump_debug(ateccx08a_config_bytes, 80);
    fido_log_print_hexdump_debug(ateccx08a_config_bytes + 80, 48);
    return true;
}

bool fido_cryptoauth_init(void)
{
    fido_log_info("fido_cryptoauth_init start");

    // 初期化処理が未実行の場合、デバイスを初期化
    if (init_device(&m_iface_config) == false) {
        return false;
    }

    // シリアル番号を取得
    if (get_cryptoauth_serial_num() == false) {
        return false;
    }

    // config情報を取得
    if (get_cryptoauth_config_bytes() == false) {
        return false;
    }

    fido_log_info("fido_cryptoauth_init success");
    return true;
}

void fido_cryptoauth_release(void)
{
    if (atcab_init_done) {
        // デバイスを解放
        atcab_release();
        atcab_init_done = false;
        fido_log_info("fido_cryptoauth_release done");
    }
}

void fido_cryptoauth_keypair_generate(uint16_t key_id)
{
    memset(public_key_raw_data, 0x00, sizeof(public_key_raw_data));

    ATCA_STATUS status = atcab_genkey(key_id, public_key_raw_data);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_keypair_generate failed: atcab_genkey(%d) returns 0x%08x", 
            key_id, status);
    } else {
        fido_log_debug("fido_cryptoauth_keypair_generate success (key_id=%d)", key_id);
    }
}

uint8_t *fido_cryptoauth_keypair_public_key(uint16_t key_id)
{
    memset(public_key_raw_data, 0x00, sizeof(public_key_raw_data));

    ATCA_STATUS status = atcab_get_pubkey(key_id, public_key_raw_data);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_crypto_keypair_public_key failed: atcab_get_pubkey(%d) returns 0x%08x", 
            key_id, status);
    }
    fido_log_debug("fido_crypto_keypair_public_key (key_id=%d):", key_id);
    fido_log_print_hexdump_debug(public_key_raw_data, sizeof(public_key_raw_data));

    return public_key_raw_data;
}

size_t fido_cryptoauth_keypair_private_key_size(void)
{
    return sizeof(public_key_raw_data);
}
