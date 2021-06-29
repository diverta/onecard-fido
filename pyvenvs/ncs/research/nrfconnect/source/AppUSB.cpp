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

#include "AppEventHandler.h"
#include "AppProcess.h"

//
// USBデバイスのステータスを管理
//
static enum usb_dc_status_code m_status;

static void AppEventHandlerCallback(void *param)
{
    (void)param;
    switch (m_status) {
        case USB_DC_CONNECTED:
            break;
        case USB_DC_CONFIGURED:
            AppProcessUSBConfigured();
            break;
        case USB_DC_DISCONNECTED:
            AppProcessUSBDisconnected();
            break;
        case USB_DC_SOF:
            break;
        default:
            break;
    }
}

static void StatusCallback(enum usb_dc_status_code status, const uint8_t *param)
{
    // AppEventHandler経由で、ステータス変更時の処理を実行
    (void)param;
    m_status = status;
    AppEventHandlerFunctionEventPost(AppEventHandlerCallback, nullptr);
}

//
// USBデバイス初期化処理
//
bool AppUSBInitialize(void)
{
    // USBデバイスを使用可能にする
    int ret = usb_enable(StatusCallback);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return false;
    }

    LOG_INF("USB initialized");
    return true;
}
