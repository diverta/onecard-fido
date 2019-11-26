/* 
 * File:   fido_cryptoauth.c
 * Author: makmorit
 *
 * Created on 2019/11/25, 14:43
 */
// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECC608A関連
//
#include "atca_iface.h"
#include "cryptoauthlib.h"

// 設定情報を保持
static ATCAIfaceCfg m_iface_config;

// シリアルナンバーを保持
static uint8_t cryptoauth_serial_num[ATCA_SERIAL_NUM_SIZE];

static void select_device(ATCAIfaceCfg *p_cfg)
{
    p_cfg->devtype               = ATECC608A;
    p_cfg->iface_type            = ATCA_I2C_IFACE;
    p_cfg->atcai2c.slave_address = 0xc0;
    p_cfg->atcai2c.bus           = 2;
    p_cfg->atcai2c.baud          = 400000;
    p_cfg->wake_delay            = 1500;
    p_cfg->rx_retries            = 20;

    *p_cfg = cfg_ateccx08a_i2c_default;
    p_cfg->devtype = ATECC608A;
}

static bool get_cryptoauth_serial_num(ATCAIfaceCfg *p_cfg)
{
    ATCA_STATUS status = atcab_init(p_cfg);
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_serial_no: atcab_init() failed with ret=0x%08x", status);
        return false;
    }

    status = atcab_read_serial_number(cryptoauth_serial_num);
    atcab_release();
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_serial_no: atcab_read_serial_number() failed with ret=0x%08x", status);
        return false;
    }
    fido_log_debug("Serial number:");
    fido_log_print_hexdump_debug(cryptoauth_serial_num, ATCA_SERIAL_NUM_SIZE);
    return true;
}

bool fido_cryptoauth_init(void)
{
    fido_log_info("fido_cryptoauth_init start");

    // ATECC608Aからシリアル番号を取得
    select_device(&m_iface_config);
    if (get_cryptoauth_serial_num(&m_iface_config)) {
        fido_log_info("fido_cryptoauth_init success");
        return true;

    } else {
        fido_log_error("fido_cryptoauth_init failed");
        return false;
    }
}
