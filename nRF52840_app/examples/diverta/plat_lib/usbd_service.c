/* 
 * File:   usbd_service.c
 * Author: makmorit
 *
 * Created on 2018/11/06, 14:21
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// for __bswap32()
#include <machine/endian.h>

#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_power.h"
#include "app_usbd.h"
#include "app_error.h"

#include "usbd_service.h"
#include "usbd_service_ccid.h"
#include "usbd_service_hid.h"

// for fido_board_delay_ms
#include "fido_board.h"

// for BOOTLOADER_DFU_START
#include "nrf_bootloader_info.h"
#include "nrf_delay.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// USB製品関連情報
//  nRF5 SDKの app_usbd_string_desc.c 内で加工され、
//  ディスクリプター応答されます。
//
uint8_t app_usbd_strings_manufacturer[] = USBD_STRINGS_MANUFACTURER;
uint8_t app_usbd_strings_product[]      = USBD_STRINGS_PRODUCT;
uint8_t app_usbd_strings_serial[16];

// Work area for update app_usbd_strings_serial
static uint32_t hwid_0;
static uint32_t hwid_1;
static char work_buf[20];

//
// USB共通処理
//
// USBイベントハンドラーの参照を待避
static void (*event_handler)(void);

void usbd_service_start(void)
{
    // USBデバイスクラスを初期化
    usbd_hid_init();
    usbd_ccid_init();

    // USBデバイスを開始
    ret_code_t ret = app_usbd_power_events_enable();
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_power_events_enable() returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("usbd_service_start() done");
}

bool usbd_service_support_bootloader_mode(void)
{
    // ブートローダーモード遷移をサポート
    return true;
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
    fido_board_delay_ms(500);

    // Get contents of the general purpose retention registers 
    // (NRF_POWER->GPREGRET*)
    uint32_t gpregret;
    uint32_t err_code = sd_power_gpregret_get(0, &gpregret);
    APP_ERROR_CHECK(err_code);

    // システムリセットを実行
    //   GPREGRETレジスターに値が設定されている場合、
    //   ブートローダーモードに遷移
    fido_board_system_reset();
}

static void usbd_user_ev_handler_custom(app_usbd_event_type_t event)
{
    if (event == APP_USBD_EVT_POWER_DETECTED) {
        // BLEペリフェラル稼働中にUSB接続された場合は、
        // ソフトデバイスを再起動し、
        // BLEペリフェラルを無効化
        (*event_handler)();
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    // アプリケーション固有の処理を実行
    usbd_user_ev_handler_custom(event);

    switch (event) {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_RESET:
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            // Allow the library to put the peripheral into sleep mode
            app_usbd_suspend_req(); 
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_STARTED:
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

void usbd_service_init(void)
{
    // ハードウェアIDを抽出
    hwid_0 = NRF_FICR->DEVICEID[0];
    hwid_1 = NRF_FICR->DEVICEID[1];
    snprintf(work_buf, sizeof(work_buf), "%08lX%08lX", __bswap32(hwid_0), __bswap32(hwid_1));

    // ハードウェアIDのうち、
    // USBD_STRINGS_SERIALで定義した文字列の長さ分を
    // シリアル番号として設定
    size_t len = strlen(USBD_STRINGS_SERIAL);
    size_t offset = 16 - len;
    memcpy(app_usbd_strings_serial, work_buf + offset, len);

    ret_code_t ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);
    while(!nrf_drv_clock_lfclk_is_running());

    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };
    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
    
    NRF_LOG_DEBUG("usbd_init() done");
}

void usbd_service_do_process(void)
{
    // HIDサービスを稼働させる
    usbd_service_hid_do_process(true);

    // CCIDサービスを稼働させる
    usbd_service_ccid_do_process();
}

void usbd_service_pwr_detect_func(void (*event_handler_)(void))
{
    // USBイベントハンドラーの参照を待避
    event_handler = event_handler_;
}