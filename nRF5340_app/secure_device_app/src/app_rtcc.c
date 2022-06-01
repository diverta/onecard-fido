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
