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

static ATECC_STATUS init_atecc_command(ATECC_DEVICE_TYPE device_type, ATECC_COMMAND command)
{
    if (command == NULL) {
        return ATECC_BAD_PARAM;
    }

    command->dt = device_type;
    command->clock_divider = 0;

    return ATECC_SUCCESS;
}

static ATECC_STATUS init_atecc_device(ATECC_IFACE_CFG *cfg, ATECC_DEVICE device)
{
    ATECC_STATUS status;

    if (cfg == NULL || device == NULL || device->mCommands == NULL || device->mIface == NULL) {
        return ATECC_BAD_PARAM;
    }

    status = init_atecc_command(cfg->devtype, device->mCommands);
    if (status != ATECC_SUCCESS) {
        return status;
    }

    status = atecc_iface_init(cfg, device->mIface);
    if (status != ATECC_SUCCESS) {
        return status;
    }

    return ATECC_SUCCESS;
}

static ATECC_STATUS release_atecc_device(ATECC_DEVICE device) 
{
    if (device == NULL) {
        return ATECC_BAD_PARAM;
    }

    return atecc_iface_release(device->mIface);
}

ATECC_STATUS atecc_device_release(void)
{
    ATECC_STATUS status = release_atecc_device(_atecc_device);
    if (status != ATECC_SUCCESS) {
        return status;
    }
    _atecc_device = NULL;

    return ATECC_SUCCESS;
}

ATECC_STATUS atecc_device_init(void)
{
    ATECC_STATUS status = ATECC_GEN_FAIL;

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
    if (status != ATECC_SUCCESS) {
        return status;
    }
    _atecc_device = &m_atecc_device;

    // ATECC608Aの分周設定
    if ((status = atecc_read_bytes_zone(ATECC_ZONE_CONFIG, 0, ATECC_CHIPMODE_OFFSET, &_atecc_device->mCommands->clock_divider, 1)) != ATECC_SUCCESS) {
        return status;
    }
    _atecc_device->mCommands->clock_divider &= ATECC_CHIPMODE_CLOCK_DIV_MASK;

    return ATECC_SUCCESS;
}
