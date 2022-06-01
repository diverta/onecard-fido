/* 
 * File:   app_rtcc.c
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:08
 */
#include <stdio.h>
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>

#include "app_rtcc_define.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_rtcc);

static const struct device *i2c_dev;

// データ送受信用の一時領域
static struct i2c_msg msgs[2];
static uint8_t read_buff[32];
static uint8_t write_buff[32];

//
// I2C write & read
//
static bool read_register(uint8_t reg_addr, uint8_t *reg_val)
{
    write_buff[0] = reg_addr;

    // Send the address to read from
    msgs[0].buf = write_buff;
    msgs[0].len = 1U;
    msgs[0].flags = I2C_MSG_WRITE;

    // Read from device. STOP after this
    msgs[1].buf = read_buff;
    msgs[1].len = 1U;
    msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

    if (i2c_transfer(i2c_dev, &msgs[0], 2, RV3028C7_ADDRESS) != 0) {
        LOG_DBG("i2c_transfer error");
        return false;
    }

    *reg_val = read_buff[0];
    return true;
}

//
// RTCCの初期化
//
bool app_rtcc_initialize(void)
{
    // 制御レジスター（Control 2 register）を参照、0x00なら正常
    uint8_t c2;
    if (read_register(RV3028C7_REG_CONTROL_2, &c2) == false) {
        return false;
    }
    if (c2 != 0x00) {
        LOG_ERR("RTCC is not available");
        return false;
    }
    return true;
}

//
// デバイスの初期化
//
static int app_rtcc_init(const struct device *dev)
{
    // I2C（i2c1）デバイス初期化
    (void)dev;
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (device_is_ready(i2c_dev) == false) {
        LOG_ERR("RTCC device is not ready");
        return -ENOTSUP;
    }

    LOG_INF("RTCC device is ready");
    return 0;
}

SYS_INIT(app_rtcc_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
