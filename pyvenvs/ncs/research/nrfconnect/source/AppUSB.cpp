/* 
 * File:   AppUSB.cpp
 * Author: makmorit
 *
 * Created on 2021/06/29, 12:41
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppUSB);

#include <usb/usb_device.h>

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
    // TODO: 仮の実装です。
    (void)param;
    LOG_INF("USB status %d", status);
}

//
// USBデバイス初期化処理
//
bool AppUSBInitialize(void)
{
    // USBデバイスを使用可能にする
    int ret = usb_enable(status_cb);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return false;
    }

    LOG_INF("USB initialized");
    return true;
}
