/* 
 * File:   atecc_device.c
 * Author: makmorit
 *
 * Created on 2020/08/11, 10:19
 */
#include "atecc_device.h"
#include "atecc_read.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// 設定情報を保持
static ATECC_IFACE_CFG m_iface_config;

ATECC_IFACE_CFG cfg_ateccx08a_i2c_default = {
    .devtype           = ATECC608A,
    .iface_type        = ATECC_I2C_IFACE,
    .i2c.slave_address = 0xC0,
    .i2c.bus           = 2,
    .i2c.baud          = 400000,
    .wake_delay        = 1500,
    .rx_retries        = 20
};

static ATECC_DEVICE _atecc_device = NULL;
static struct atecc_command m_atecc_command;
static struct atecc_iface   m_atecc_iface;
static struct atecc_device  m_atecc_device;

ATECC_DEVICE atecc_device_ref(void)
{
    return _atecc_device;
}

static bool init_atecc_command(ATECC_DEVICE_TYPE device_type, ATECC_COMMAND command)
{
    if (command == NULL) {
        fido_log_error("init_atecc_command failed: BAD_PARAM");
        return false;
    }

    command->dt = device_type;
    command->clock_divider = 0;

    return true;
}

static bool init_atecc_device(ATECC_IFACE_CFG *cfg, ATECC_DEVICE device)
{
    bool status;

    if (cfg == NULL || device == NULL || device->mCommands == NULL || device->mIface == NULL) {
        fido_log_error("init_atecc_device failed: BAD_PARAM");
        return false;
    }

    status = init_atecc_command(cfg->devtype, device->mCommands);
    if (status == false) {
        return status;
    }

    status = atecc_iface_init(cfg, device->mIface);
    if (status == false) {
        return status;
    }

    return true;
}

static bool release_atecc_device(ATECC_DEVICE device) 
{
    if (device == NULL) {
        fido_log_error("release_atecc_device failed: BAD_PARAM");
        return false;
    }

    return atecc_iface_release(device->mIface);
}

bool atecc_device_release(void)
{
    bool status = release_atecc_device(_atecc_device);
    if (status == false) {
        return status;
    }
    _atecc_device = NULL;

    return true;
}

bool atecc_device_init(void)
{
    bool status = false;

    // デバイス設定は、ライブラリーのデフォルトを採用
    ATECC_IFACE_CFG *cfg = &m_iface_config;
    *cfg = cfg_ateccx08a_i2c_default;

    // If a device has already been initialized, release it
    if (_atecc_device) {
        atecc_device_release();
    }

    m_atecc_device.mCommands = &m_atecc_command;
    m_atecc_device.mIface = &m_atecc_iface;

    // I2C初期設定、TWI有効化
    status = init_atecc_device(cfg, &m_atecc_device);
    if (status == false) {
        return status;
    }
    _atecc_device = &m_atecc_device;

    // ATECC608Aの分周設定
    if ((status = atecc_read_bytes_zone(ATECC_ZONE_CONFIG, 0, ATECC_CHIPMODE_OFFSET, &_atecc_device->mCommands->clock_divider, 1)) == false) {
        return status;
    }
    _atecc_device->mCommands->clock_divider &= ATECC_CHIPMODE_CLOCK_DIV_MASK;

    return true;
}
