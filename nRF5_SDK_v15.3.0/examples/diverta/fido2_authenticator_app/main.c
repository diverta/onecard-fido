#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_timer.h"
#include "app_usbd.h"
#include "ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_lesc.h"
#include "nrf_pwr_mgmt.h"
#include "fds.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// FIDO Authenticator固有の処理
#include "fido_ble_event.h"
#include "fido_ble_peripheral.h"
#include "fido_ble_peripheral_timer.h"
#include "fido_hid_channel.h"
#include "usbd_hid_service.h"
#include "ctap2_client_pin.h"
#include "nfc_service.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for fido_button_timers_init
#include "fido_board.h"

// for FDS event handle
#include "fido_flash.h"

#define APP_BLE_CONN_CFG_TAG                1           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO               3           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define DEAD_BEEF                           0xDEADBEEF  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

NRF_BLE_GATT_DEF(m_gatt);                               /**< GATT module instance. */

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // FIDO Authenticator固有のタイマー機能
    fido_button_timers_init();
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, fido_ble_peripheral_evt_handler, NULL);
}


static void flash_storage_init(void)
{
    // FDSを初期化
    ret_code_t err_code = fds_init();
    APP_ERROR_CHECK(err_code);

    // FDSイベント発生後に実行される
    // FIDO Authenticator固有の処理を
    // fds_registerで登録
    fido_flash_fds_event_register();
}


/**@brief Function for initializing the GATT library. */
static void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, fido_ble_peripheral_gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_lesc_request_handler();
    APP_ERROR_CHECK(err_code);

#ifdef BOARD_PCA10056
    // nRF52840 DKで開発時のみ、ログが出力されるようにする
    if (NRF_LOG_PROCESS()) {
        return;
    }
#endif

    nrf_pwr_mgmt_run();
}

/**
 * @brief Function for handling USBD specific event
 *
 * @param event     USBD library event.
 * */
static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    if (event == APP_USBD_EVT_POWER_DETECTED) {
        if (fido_ble_peripheral_mode()) {
            // BLEペリフェラル稼働中にUSB接続された場合は、
            // ソフトデバイスを再起動
            NVIC_SystemReset();
        }
    }    
    if (event == APP_USBD_EVT_STOPPED) {
        // 給電が継続している場合は、USBを無効化したのちに
        // ソフトデバイスを再起動
        app_usbd_disable();
        NVIC_SystemReset();
    }
}

static void application_init(void)
{
    // アプリケーションで使用するボタンの設定
    fido_button_init();

    // アプリケーションで使用するCIDを初期化
    fido_hid_channel_initialize_cid();

    // PINトークンとキーペアを再生成
    ctap2_client_pin_init();
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    usbd_init();
    timers_init();
    power_management_init();
    ble_stack_init();
    flash_storage_init();

    // BLE関連の初期化
    gatt_init();
    fido_ble_peripheral_init();

    // USB HIDデバイスクラスを初期化
    usbd_hid_init(usbd_user_ev_handler);

    // NFC関連の初期化（機能閉塞中です）
    nfc_service_init(true);
    
    // BLEペリフェラル始動タイマーを開始
    fido_ble_peripheral_timer_start();
    NRF_LOG_INFO("Diverta FIDO Authenticator application started.");

    // アプリケーション固有の初期化処理
    application_init();

    // Enter main loop.
    for (;;) {
        // 業務処理を実行
        usbd_hid_do_process();
        fido_ble_do_process();
        idle_state_handle();
    }
}
