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
#include <usb/class/usb_hid.h>

#include "AppEventHandler.h"
#include "AppProcess.h"

// 作業領域
static uint8_t  m_report[64];
static uint32_t m_report_size;

// 関数プロトタイプ
static void int_in_ready_cb(const struct device *dev);
static void int_out_ready_cb(const struct device *dev);
static void on_idle_cb(const struct device *dev, uint16_t report_id);

//
// USBデバイスのステータスを管理
//
static enum usb_dc_status_code m_status;

static void AppEventHandlerCallback(void *param)
{
    if (param == nullptr) {
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

    } else if (param == (void *)int_in_ready_cb) {
        AppProcessHIDReportSent();

    } else if (param == (void *)int_out_ready_cb) {
        AppProcessHIDReportReceived(m_report, m_report_size);
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

//
// USB HIDインターフェース
//
// HID I/Fディスクリプター
static const uint8_t hid_report_desc[] = {
    0x06, 0xd0, 0xf1, /* Usage Page (FIDO Alliance),         */
    0x09, 0x01,       /* Usage (FIDO USB HID),               */
    0xa1, 0x01,       /*  Collection (Application),          */
    0x09, 0x20,       /*   Usage (Input Report Data),        */
    0x15, 0x00,       /*   Logical Minimum (0),              */
    0x26, 0xff, 0x00, /*   Logical Maximum (255),            */
    0x75, 0x08,       /*   Report Size (8),                  */
    0x95, 0x40,       /*   Report Count (64 bytes),          */
    0x81, 0x02,       /*   Input (Data, Variable, Absolute)  */
    0x09, 0x21,       /*   Usage (Output Report Data),       */
    0x15, 0x00,       /*   Logical Minimum (0),              */
    0x26, 0xff, 0x00, /*   Logical Maximum (255),            */
    0x75, 0x08,       /*   Report Size (8),                  */
    0x95, 0x40,       /*   Report Count (64 bytes),          */
    0x91, 0x02,       /*   Output (Data, Variable, Absolute) */
    0xc0,             /* End Collection                      */
};

//
// USB HIDインターフェース初期化
//
static const struct device *hdev;
static const struct hid_ops ops = {
    .on_idle         = on_idle_cb,
    .int_in_ready    = int_in_ready_cb,
    .int_out_ready   = int_out_ready_cb
};

static int composite_pre_init(const struct device *dev)
{
    (void)dev;
    hdev = device_get_binding("HID_0");
    if (hdev == NULL) {
        LOG_ERR("Cannot get USB HID Device");
        return -ENODEV;
    }

    usb_hid_register_device(hdev, hid_report_desc, sizeof(hid_report_desc), &ops);
    return usb_hid_init(hdev);
}

SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

//
// USB HIDインターフェースからのコールバック
//
static void int_in_ready_cb(const struct device *dev)
{
    // フレーム送信完了時の処理
    (void)dev;
    memset(m_report, 0, sizeof(m_report));

    // イベント管理に通知
    AppEventHandlerFunctionEventPost(AppEventHandlerCallback, (void *)int_in_ready_cb);
}

static void int_out_ready_cb(const struct device *dev)
{
    // フレーム受信完了時の処理
    (void)dev;
    int ret = hid_int_ep_read(dev, m_report, sizeof(m_report), &m_report_size);
    if (ret != 0) {
        LOG_ERR("hid_int_ep_read returns %d", ret);
        return;
    }

    // イベント管理に通知
    AppEventHandlerFunctionEventPost(AppEventHandlerCallback, (void *)int_out_ready_cb);
}

static void on_idle_cb(const struct device *dev, uint16_t report_id)
{
    (void)dev;
    (void)report_id;
}
