/* 
 * File:   app_usb_hid.c
 * Author: makmorit
 *
 * Created on 2021/05/04, 11:16
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <errno.h>
#include <init.h>
#include <usb/usb_device.h>
#include <usb/class/usb_hid.h>

#include "app_event.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_usb_hid);

#define LOG_DEBUG_REPORT        false
#define LOG_DEBUG_INPUT_REPORT  false
#define WRITE_REPORT_ON_IDLE_CB false

// HIDデバイスのインスタンス
static const struct device *hdev;

//
// HID I/Fディスクリプター（FIDO機能用）
//
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

// 送受信データ格納用
static uint8_t m_report[64];

//
// USB HIDインターフェースからのコールバック
//
static void int_in_ready_cb(const struct device *dev)
{
    // フレーム送信完了時の処理
    (void)dev;
    memset(m_report, 0, sizeof(m_report));

    // データ処理スレッドに通知
    app_event_notify_for_data(DATEVT_HID_REPORT_SENT, NULL, 0);
}

static void int_out_ready_cb(const struct device *dev)
{
    // フレーム受信完了時の処理
    (void)dev;

    uint32_t ret_bytes;
    int ret = hid_int_ep_read(dev, m_report, sizeof(m_report), &ret_bytes);
    if (ret != 0) {
        LOG_ERR("hid_int_ep_read returns %d", ret);
        return;
    }

#if LOG_DEBUG_REPORT
    LOG_DBG("hid_int_ep_read done (%d bytes)", ret_bytes);
    LOG_HEXDUMP_DBG(m_report, ret_bytes, "Output report");
#endif
    // データ処理スレッドに引き渡し
    app_event_notify_for_data(DATEVT_HID_REPORT_RECEIVED, m_report, sizeof(m_report));
}

static void on_idle_cb(const struct device *dev, uint16_t report_id)
{
    (void)report_id;
    
#if WRITE_REPORT_ON_IDLE_CB
    m_report[0] = 0x00;
    m_report[1] = 0xeb;

    int wrote;
    int ret = hid_int_ep_write(dev, m_report, 2, &wrote);
    if (ret != 0) {
        LOG_ERR("hid_int_ep_write returns %d", ret);
    }
#endif
}

//
// USB HIDインターフェース初期化
//
static const struct hid_ops ops = {
    .int_in_ready    = int_in_ready_cb,
    .int_out_ready   = int_out_ready_cb,
    .on_idle         = on_idle_cb,
};

static int composite_pre_init(const struct device *dev)
{
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
// USBデバイスからのコールバック
//
void app_usb_hid_configured(const uint8_t *param)
{
    // 内部変数をクリア
    (void)param;
    memset(m_report, 0, sizeof(m_report));
}

bool app_usb_hid_send_report(uint8_t *data, size_t size)
{
    // データを設定
    memcpy(m_report, data, size);

    // USBデバイスにフレーム送信
    int ret_bytes;
    int ret = hid_int_ep_write(hdev, m_report, size, &ret_bytes);
    if (ret != 0) {
        LOG_ERR("hid_int_ep_write returns %d", ret);
        return false;
    }

#if LOG_DEBUG_INPUT_REPORT
    LOG_DBG("hid_int_ep_write done (%d bytes)", ret_bytes);
    LOG_HEXDUMP_DBG(m_report, ret_bytes, "Input report");
#endif
    return true;
}
