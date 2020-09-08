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

// for application initialize
#include "application_init.h"

// FIDO Authenticator固有の処理
#include "usbd_service.h"
#include "nfc_service.h"
#include "ble_service_common.h"

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
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_service_common_evt_handler, NULL);
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

    err_code = nrf_ble_gatt_init(&m_gatt, ble_service_common_gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
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

#if NRF_LOG_BACKEND_UART_ENABLED
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
        // BLEペリフェラル稼働中にUSB接続された場合は、
        // ソフトデバイスを再起動し、
        // BLEペリフェラルを無効化
        ble_service_common_disable_peripheral();
    }
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    usbd_service_init(usbd_user_ev_handler);
    timers_init();
    power_management_init();
    flash_storage_init();

    // BLE関連の初期化
    ble_stack_init();
    gatt_init();
    ble_service_common_init();

    // USBデバイスを開始
    usbd_service_start();

    // NFC関連の初期化（機能閉塞中です）
    nfc_service_init(true);

    // アプリケーション稼働に必要な初期化処理を開始
    application_init_start();

    // Enter main loop.
    for (;;) {
        // USBデバイス処理を実行
        while (app_usbd_event_queue_process());

        // 業務処理を実行
        application_main();

        // その他デバイス固有の処理を実行
        idle_state_handle();
    }
}
