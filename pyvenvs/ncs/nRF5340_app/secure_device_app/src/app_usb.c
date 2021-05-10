/* 
 * File:   app_usb.c
 * Author: makmorit
 *
 * Created on 2021/05/04, 11:16
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <usb/usb_device.h>

#include "app_event.h"
#include "app_usb_hid.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_usb);

//
// USBデバイスのステータスを管理
//
static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
    (void)param;
    switch (status) {
        case USB_DC_CONNECTED:
            app_event_notify(APEVT_USB_CONNECTED);
            break;
        case USB_DC_CONFIGURED:
            app_event_notify(APEVT_USB_CONFIGURED);
            app_usb_hid_configured(param);
            break;
        case USB_DC_DISCONNECTED:
            app_event_notify(APEVT_USB_DISCONNECTED);
            break;
        case USB_DC_SOF:
            break;
    default:
        break;
    }
}

//
// USBデバイス初期処理
//
void app_usb_initialize(void)
{
    // USBデバイスを使用可能にする
    int ret = usb_enable(status_cb);
    if (ret != 0) {
        LOG_ERR("Failed to enable USB");
        return;
    }

    LOG_INF("USB initialized");
}
