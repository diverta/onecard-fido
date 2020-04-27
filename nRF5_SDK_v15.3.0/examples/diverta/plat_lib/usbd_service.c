/* 
 * File:   usbd_service.c
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"
#include "app_usbd_hid_generic.h"
#include "app_error.h"

#include "usbd_service.h"
#include "usbd_service_cdc.h"
#include "fido_hid_channel.h"
#include "fido_hid_send.h"
#include "fido_hid_receive.h"

// for BOOTLOADER_DFU_START
#include "nrf_bootloader_info.h"
#include "nrf_delay.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug hid report
#define NRF_LOG_HEXDUMP_DEBUG_REPORT false

//
// HID関連
//
/**
 * @brief Number of reports defined in report descriptor.
 */
#define REPORT_IN_QUEUE_SIZE    1

/**
 * @brief Size of maximum output report. HID generic class will reserve
 *        this buffer size + 1 memory space. 
 *
 * Maximum value of this define is 63 bytes. Library automatically adds
 * one byte for report ID. This means that output report size is limited
 * to 64 bytes.
 */
#define REPORT_OUT_MAXSIZE  63

/**
 * @brief List of HID generic class endpoints.
 * */
#define ENDPOINT_LIST()                                      \
(                                                            \
        HID_GENERIC_EPIN,                                    \
        HID_GENERIC_EPOUT                                    \
)

/**
 * @brief User event handler.
 * */
static void hid_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event);


#define APP_USBD_HID_FIDO_REPORT_DESC(bcnt) {             \
    0x06, 0xd0, 0xf1, /* Usage Page (FIDO Alliance),         */     \
    0x09, 0x01,       /* Usage (FIDO USB HID),               */     \
    0xa1, 0x01,       /*  Collection (Application),          */     \
    0x09, 0x20,       /*   Usage (Input Report Data),        */     \
    0x15, 0x00,       /*   Logical Minimum (0),              */     \
    0x26, 0xff, 0x00, /*   Logical Maximum (255),            */     \
    0x75, 0x08,       /*   Report Size (8),                  */     \
    0x95, bcnt,       /*   Report Count (bcnt),              */     \
    0x81, 0x02,       /*   Input (Data, Variable, Absolute)  */     \
    0x09, 0x21,       /*   Usage (Output Report Data),       */     \
    0x15, 0x00,       /*   Logical Minimum (0),              */     \
    0x26, 0xff, 0x00, /*   Logical Maximum (255),            */     \
    0x75, 0x08,       /*   Report Size (8),                  */     \
    0x95, bcnt,       /*   Report Count (bcnt),              */     \
    0x91, 0x02,       /*   Output (Data, Variable, Absolute) */     \
    0xc0,             /* End Collection                      */     \
}

/**
 * @brief Reuse USB HID report descriptor for HID generic class
 */
APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(report_desc, APP_USBD_HID_FIDO_REPORT_DESC(64));

static const app_usbd_hid_subclass_desc_t * reps[] = {&report_desc};

/*lint -save -e26 -e64 -e123 -e505 -e651*/

/**
 * @brief Global HID generic instance
 */
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_generic,
                                HID_GENERIC_INTERFACE,
                                hid_user_ev_handler,
                                ENDPOINT_LIST(),
                                reps,
                                REPORT_IN_QUEUE_SIZE,
                                REPORT_OUT_MAXSIZE,
                                APP_USBD_HID_SUBCLASS_NONE,
                                APP_USBD_HID_PROTO_GENERIC);

/**
 * @brief Mark the ongoing transmission
 *
 * Marks that the report buffer is busy and cannot be used until transmission finishes
 * or invalidates (by USB reset or suspend event).
 */
static bool m_report_pending;
static bool m_hid_report_received = false;

static void usbd_output_report_received(app_usbd_class_inst_t const * p_inst)
{
    // Output reportが格納されている領域を取得
    app_usbd_hid_generic_t const *p_hid = app_usbd_hid_generic_class_get(p_inst);
    app_usbd_hid_report_buffer_t const *rep_buf = 
        app_usbd_hid_rep_buff_out_get(&p_hid->specific.inst.hid_inst);

#if NRF_LOG_HEXDUMP_DEBUG_REPORT
    NRF_LOG_DEBUG("Output Report: %d bytes", rep_buf->size);
    NRF_LOG_HEXDUMP_DEBUG(rep_buf->p_buff, rep_buf->size);
#endif

    // Output reportから受信フレームを取得し、
    // request_frame_bufferに格納
    // 受信フレーム数は、request_frame_numberに設定される
    m_hid_report_received = fido_hid_receive_request_frame(
        rep_buf->p_buff, rep_buf->size);
}

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 * */
static void hid_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event)
{
    switch (event)
    {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
        {
            // Output reportの内容を取り出す
            usbd_output_report_received(p_inst);
            break;
        }
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
        {
            fido_hid_send_input_report_complete();
            m_report_pending = false;
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
        {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_DEBUG("SET_BOOT_PROTO");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
        {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_DEBUG("SET_REPORT_PROTO");
            break;
        }
        default:
            break;
    }
}

static ret_code_t idle_handle(app_usbd_class_inst_t const * p_inst, uint8_t report_id)
{
    switch (report_id) {
        case 0:
            NRF_LOG_DEBUG("idle_handle(0) called");
            break;
        default:
            return NRF_ERROR_NOT_SUPPORTED;
    }
    return NRF_SUCCESS;
}

static void usbd_hid_init(void)
{ 
    ret_code_t ret;
    app_usbd_class_inst_t const * class_inst_generic;
    class_inst_generic = app_usbd_hid_generic_class_inst_get(&m_app_hid_generic);

    ret = hid_generic_idle_handler_set(class_inst_generic, idle_handle);
    APP_ERROR_CHECK(ret);

    ret = app_usbd_class_append(class_inst_generic);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_class_append(class_inst_generic) returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);
}

void usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    // 64バイトのInput reportを送信
    app_usbd_class_inst_t const *p_inst = 
        app_usbd_hid_generic_class_inst_get(&m_app_hid_generic);
    app_usbd_hid_generic_t const *p_hid = app_usbd_hid_generic_class_get(p_inst);
    ret_code_t ret = app_usbd_hid_generic_in_report_set(p_hid, buffer_for_send, size);    
    APP_ERROR_CHECK(ret);

    m_report_pending = true;
    
#if NRF_LOG_HEXDUMP_DEBUG_REPORT
    NRF_LOG_DEBUG("Input report: %d bytes", size);
    NRF_LOG_HEXDUMP_DEBUG(buffer_for_send, size);
#endif
}

//
// USB共通処理
//
// USBイベントハンドラーの参照を待避
static void (*event_handler)(app_usbd_event_type_t event);

void usbd_service_start(void)
{
    // USBデバイスクラスを初期化
    usbd_hid_init();
    
    // USBデバイスを開始
    ret_code_t ret = app_usbd_power_events_enable();
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_power_events_enable() returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("usbd_service_start() done");
}

void usbd_service_stop_for_bootloader(void)
{
    // ブートローダーモードに遷移させるため、
    // GPREGRETレジスターにその旨の値を設定
    // 
    // Write BOOTLOADER_DFU_START (0xb1) 
    // to the the GPREGRET REGISTER and reset the chip
    uint32_t err_code = sd_power_gpregret_set(0, BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);
    
    // USBDサービスを停止
    app_usbd_stop();
}

static void usbd_service_stopped(void)
{
    // USBを無効化
    app_usbd_disable();
    nrf_delay_ms(500);

    // Get contents of the general purpose retention registers 
    // (NRF_POWER->GPREGRET*)
    uint32_t gpregret;
    uint32_t err_code = sd_power_gpregret_get(0, &gpregret);
    APP_ERROR_CHECK(err_code);
    
    if (gpregret == BOOTLOADER_DFU_START) {
        // GPREGRETレジスターに値が設定されている場合、
        // ブートローダーモードに遷移させる
        // （ソフトデバイス経由でリセットを実行）
        sd_nvic_SystemReset();

    } else {
        // リセットを実行
        NVIC_SystemReset();
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    // アプリケーション固有の処理を実行
    (*event_handler)(event);

    switch (event) {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_RESET:
            m_report_pending = false;
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            m_report_pending = false;
            // Allow the library to put the peripheral into sleep mode
            app_usbd_suspend_req(); 
            break;
        case APP_USBD_EVT_DRV_RESUME:
            m_report_pending = false;
            break;
        case APP_USBD_EVT_STARTED:
            m_report_pending = false;
            break;
        case APP_USBD_EVT_STOPPED:
            usbd_service_stopped();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_DEBUG("USB power detected");
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_DEBUG("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_DEBUG("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}

void usbd_service_init(void (*event_handler_)(app_usbd_event_type_t event))
{
    ret_code_t ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);
    while(!nrf_drv_clock_lfclk_is_running());

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };
    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    // USBイベントハンドラーの参照を待避
    event_handler = event_handler_;
    
    NRF_LOG_DEBUG("usbd_init() done");
}

void usbd_service_do_process(void)
{
    // USBデバイス処理を実行する
    while (app_usbd_event_queue_process());

    if (usbd_cdc_port_is_open()) {
        // CDCサービスから受信データがあった場合、
        // CDC関連処理を実行
        usbd_service_cdc_do_process();

        // 受信されたHIDリクエストを無効化
        m_hid_report_received = false;

    } else {
        // 仮想COMポートから切断されている場合は
        // HIDサービスを稼働させる
        if (m_hid_report_received) {
            // HIDサービスから受信データがあった場合、
            // FIDO USB HIDサービスを実行
            m_hid_report_received = false;
            fido_hid_receive_on_request_received();
        }
    }
}
