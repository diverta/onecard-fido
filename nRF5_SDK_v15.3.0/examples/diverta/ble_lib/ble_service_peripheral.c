/* 
 * File:   ble_service_peripheral.c
 * Author: makmorit
 *
 * Created on 2019/02/11, 15:04
 */
#include "nordic_common.h"
#include "nrf.h"
#include "ble.h"
#include "ble_advertising.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "app_timer.h"
#include "nrf_ble_qwr.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_peripheral
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// FIDO Authenticator固有の処理
#include "fido_ble_service.h"
#include "fido_ble_event.h"

#include "fido_platform.h"
#include "fido_board.h"

#define APP_ADV_INTERVAL                    300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_DURATION                    18000                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */

#define APP_BLE_CONN_CFG_TAG                1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define FIRST_CONN_PARAMS_UPDATE_DELAY      APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY       APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT        3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

NRF_BLE_QWR_DEF(m_qwr);                                             /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                 /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;            /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids[] =                                   /**< Universally unique service identifiers. */
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE,   BLE_UUID_TYPE_BLE},
    {BLE_UUID_U2F_SERVICE,                  BLE_UUID_TYPE_BLE}
};

// BLEペリフェラルモードかどうかを保持
static bool ble_peripheral_mode = false;

bool fido_ble_peripheral_mode(void)
{
    return ble_peripheral_mode;
}

void fido_ble_peripheral_advertising_start(void)
{
    // アドバタイジングをスタートさせた場合は
    // BLEペリフェラルモードに移行
    ble_peripheral_mode = true;

    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEBUG("Advertising started");
}

static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};
    ble_dis_init_t     dis_init;
    
    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);
    
    // Initialize Device Information Service.
    memset(&dis_init, 0, sizeof(dis_init));

    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, (char *)MANUFACTURER_NAME);

    // BLE U2Fサービスに必要な属性を追加
    ble_srv_ascii_to_utf8(&dis_init.model_num_str    , (char *)MODEL_NUM);
	ble_srv_ascii_to_utf8(&dis_init.fw_rev_str       , (char *)FW_REV);

    dis_init.dis_char_rd_sec = SEC_JUST_WORKS;

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);

    // FIDO Authenticator固有のサービスを追加設定
    fido_ble_services_init();
}

static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}

static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    // FIDO Authenticator固有の処理
    fido_ble_sleep_mode_enter();

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("BLE: Fast advertising.");
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}

static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = false;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    // FIDO Authenticator固有の設定
    fido_ble_advertising_init(&init);
    
    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

void fido_ble_peripheral_init(void)
{
    // ペリフェラルデバイスとしての
    // 各種初期処理を実行
    services_init();
    conn_params_init();

    // advertising_initの実行は
    // peer_manager_init実行後とする
    //   アドバタイジング設定の前に
    //   ペアリングモードをFDSから
    //   取得する必要があるための措置
    advertising_init();
    NRF_LOG_INFO("BLE peripheral initialized");

    // ペアリングモードの場合は
    // このタイミングで黄色LEDを点灯させる
    if (fido_ble_pairing_mode_get()) {
        fido_status_indicator_pairing_mode();
    }
}

void fido_ble_peripheral_start(void)
{
    // USB接続・HIDサービスが始動していない場合は
    // アドバタイジングを開始させ、
    // BLEペリフェラル・モードに遷移
    fido_ble_peripheral_advertising_start();

    if (fido_ble_pairing_mode_get() == false) {
        // LED制御をアイドル中に変更
        //   ペアリングモードでない場合は
        //   青色LEDを秒間２回点滅
        fido_status_indicator_idle();
    }
}

//
// BLE GAPイベント関連処理
//
void ble_peripheral_gap_connected(ble_evt_t const *p_ble_evt)
{
    if (ble_peripheral_mode == false) {
        return;
    }
    m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    ret_code_t err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
    APP_ERROR_CHECK(err_code);
}

void ble_peripheral_gap_disconnected(ble_evt_t const *p_ble_evt)
{
    if (ble_peripheral_mode == false) {
        return;
    }
    m_conn_handle = BLE_CONN_HANDLE_INVALID;
}

//
// BLEペリフェラル始動判定用タイマー
//
#include "ble_service_common.h"

#define TIMER_MSEC 1000

APP_TIMER_DEF(m_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void timeout_handler(void *p_context)
{
    ble_service_common_start_peripheral(p_context);
}

static void timer_terminate(void)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // タイマーを停止
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_timer_id) returns %d ", err_code);
    }
}

static bool timer_init(void)
{
    if (app_timer_created == true) {
        return app_timer_created;
    }

    // タイマーを生成
    uint32_t err_code;
    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(timeout_handler) returns %d ", err_code);
        app_timer_created = false;
        return app_timer_created;
    }
    app_timer_created = true;

    // タイマーが既にスタートしている場合は停止させる
    timer_terminate();

    return app_timer_created;
}

void fido_ble_peripheral_timer_start(void)
{
    // タイマー生成・停止
    if (timer_init() == false) {
        return;
    }

    // タイマー開始
    uint32_t err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(TIMER_MSEC), NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}
