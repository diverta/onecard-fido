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
// OSディスクリプターを保持
//
static const uint8_t msos2_descriptor[] = {
    // Microsoft OS 2.0 descriptor set header (table 10)
    0x0A, 0x00,             // Descriptor size (10 bytes)
    0x00, 0x00,             // MS OS 2.0 descriptor set header
    0x00, 0x00, 0x03, 0x06, // Windows version (8.1) (0x06030000)
    0xB2, 0x00,             // Size, MS OS 2.0 descriptor set

    // Microsoft OS 2.0 configuration subset header
    0x08, 0x00,             // Descriptor size (8 bytes)
    0x01, 0x00,             // MS OS 2.0 configuration subset header
    0x00,                   // bConfigurationValue
    0x00,                   // Reserved
    0xA8, 0x00,             // Size, MS OS 2.0 configuration subset

    // Microsoft OS 2.0 function subset header
    0x08, 0x00,             // Descriptor size (8 bytes)
    0x02, 0x00,             // MS OS 2.0 function subset header
    0x01,                   // First interface number
    0x00,                   // Reserved
    0xA0, 0x00,             // Size, MS OS 2.0 function subset

    // Microsoft OS 2.0 compatible ID descriptor (table 13)
    0x14, 0x00,             // wLength
    0x03, 0x00,             // MS_OS_20_FEATURE_COMPATIBLE_ID
    'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x84, 0x00,             // wLength:
    0x04, 0x00,             // wDescriptorType: MS_OS_20_FEATURE_REG_PROPERTY: 0x04 (Table 9)
    0x07, 0x00,             // wPropertyDataType: REG_MULTI_SZ (Table 15)
    0x2A, 0x00,             // wPropertyNameLength:
    // bPropertyName: "DeviceInterfaceGUID"
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00,
    'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00,
    'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    0x50, 0x00,             // wPropertyDataLength
    // bPropertyData: "{244eb29e-e090-4e49-81fe-1f20f8d3b8f4}"
    '{', 0x00, '2', 0x00, '4', 0x00, '4', 0x00, 'E', 0x00, 'B', 0x00, '2', 0x00,
    '9', 0x00, 'E', 0x00, '-', 0x00, 'E', 0x00, '0', 0x00, '9', 0x00, '0', 0x00,
    '-', 0x00, '4', 0x00, 'E', 0x00, '4', 0x00, '9', 0x00, '-', 0x00, '8', 0x00,
    '1', 0x00, 'F', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, 'F', 0x00, '2', 0x00,
    '0', 0x00, 'F', 0x00, '8', 0x00, 'D', 0x00, '3', 0x00, 'B', 0x00, '8', 0x00,
    'F', 0x00, '4', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

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

//
// USBデバイス停止処理
//
bool app_usb_deinitialize(void)
{
    // USBを停止
    int ret = usb_disable();
    if (ret != 0) {
        LOG_ERR("Failed to disable USB");
        return false;
    }

    LOG_INF("USB deinitialized");
    return true;
}
