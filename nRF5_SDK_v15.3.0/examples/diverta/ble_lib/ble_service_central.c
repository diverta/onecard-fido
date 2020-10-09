/* 
 * File:   ble_service_central.c
 * Author: makmorit
 *
 * Created on 2019/10/02, 14:47
 */
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_gap.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_scan.h"
#include "peer_manager.h"
#include "peer_data_storage.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_central
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define APP_BLE_CONN_CFG_TAG 1
NRF_BLE_SCAN_DEF(m_scan);

#include "ble_service_central.h"
#include "ble_service_central_stat.h"
#include "ble_service_common.h"

// for fido_status_indicator
#include "fido_platform.h"

// スキャン終了後に実行される関数の参照を保持
static void (*resume_function)(void);

//
// スキャン用タイマー
//
#include "app_timer.h"
APP_TIMER_DEF(m_scan_timer_id);
static bool scan_timer_created = false;
static bool scan_timer_started = false;

static void scan_timeout_handler(void *p_context)
{
    // スキャンを停止
    ble_service_central_scan_stop();
}

static ret_code_t scan_timer_init(void)
{
    if (scan_timer_created) {
        return NRF_SUCCESS;
    }

    // スキャンタイムアウト監視用タイマーを生成
    ret_code_t err_code = app_timer_create(&m_scan_timer_id, APP_TIMER_MODE_SINGLE_SHOT, scan_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_scan_timer_id) returns %d ", err_code);
    }
    
    scan_timer_created = true;
    return err_code;
}

static void scan_timer_stop(void)
{
    // スキャンタイムアウト監視用タイマーを停止
    if (scan_timer_started) {
        app_timer_stop(m_scan_timer_id);
        scan_timer_started = false;
    }
}

static void scan_timer_start(uint32_t timeout_msec, void *p_context)
{
    // タイマー生成
    ret_code_t err_code = scan_timer_init();
    if (err_code != NRF_SUCCESS) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    scan_timer_stop();

    // スキャンタイムアウト監視用タイマーを開始
    err_code = app_timer_start(m_scan_timer_id, APP_TIMER_TICKS(timeout_msec), p_context);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_scan_timer_id) returns %d ", err_code);
    } else {
        scan_timer_started = true;
    }
}

static void scan_evt_handler(scan_evt_t const *p_scan_evt)
{
    ret_code_t err_code;

    switch(p_scan_evt->scan_evt_id) {
        case NRF_BLE_SCAN_EVT_FILTER_MATCH:
            NRF_LOG_DEBUG("Scan filter matched");
            break;

        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR:
            err_code = p_scan_evt->params.connecting_err.err_code;
            NRF_LOG_ERROR("Scan connecting error: code=0x%02x", err_code);
            break;

        case NRF_BLE_SCAN_EVT_CONNECTED:
            // Scan is automatically stopped by the connection.
            break;

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:
            NRF_LOG_ERROR("Scan timed out.");
            break;

         default:
             break;
    }
}

static void scan_init(void)
{
    ret_code_t          err_code;
    nrf_ble_scan_init_t init_scan;

    memset(&init_scan, 0, sizeof(init_scan));

    init_scan.connect_if_match = false;
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;

    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);
    APP_ERROR_CHECK(err_code);
}

void ble_service_central_init(void)
{
    scan_init();
    NRF_LOG_DEBUG("BLE central initialized");
}

void ble_service_central_scan_start(uint32_t timeout_msec, void (*_resume_function)(void))
{
    // 統計情報を初期化
    ble_service_central_stat_info_init();

    // スキャンをスタート
    ret_code_t ret = nrf_ble_scan_start(&m_scan);
    APP_ERROR_CHECK(ret);

    // 赤色LED点滅を開始
    fido_status_indicator_ble_scanning();

    if (timeout_msec == 0) {
        // タイムアウトが無指定の場合
        // タイマーが既にスタートしている場合は停止させる
        scan_timer_stop();
    } else {
        // タイムアウトを指定し、
        // スキャンタイマーをスタート
        scan_timer_start(timeout_msec, NULL);
    }

    // スキャン終了後に実行される関数の参照を退避
    resume_function = _resume_function;
    
    // スキャン中の旨を通知
    NRF_LOG_DEBUG("Scan started");
}

void ble_service_central_scan_stop(void)
{
    // 赤色LED点滅を停止（アイドル状態表示に戻す）
    fido_status_indicator_idle();

    // タイマーが既にスタートしている場合は停止させる
    scan_timer_stop();

    // Stop scanning.
    nrf_ble_scan_stop();
    NRF_LOG_DEBUG("Scan stopped");

    if (resume_function != NULL) {
        // スキャン終了後に実行される関数を実行
        (*resume_function)();
    }
}

//
// 指定のBluetoothアドレスをもつ
// デバイスに対し、接続／切断要求を行う
//
// 接続完了後に実行される関数の参照を保持
static void (*resume_func_after_conn)(bool);
// 接続中のハンドルを保持
static uint16_t conn_handle;
// ペアリング済みかどうかを保持
static bool already_paired;
// 接続先のBluetoothアドレスを保持
static ble_gap_addr_t bluetooth_addr;

static void init_for_request_connection(void)
{
    // 初期化
    resume_func_after_conn = NULL;
    conn_handle = BLE_CONN_HANDLE_INVALID;
    already_paired = false;
    memset(&bluetooth_addr, 0, sizeof(ble_gap_addr_t));
}

bool ble_service_central_request_connection(ble_gap_addr_t *p_addr, void (*_resume_function)(bool))
{
    // 初期化
    init_for_request_connection();

    // For readability.
    ble_gap_scan_params_t const *p_scan_params = &m_scan.scan_params;
    ble_gap_conn_params_t const *p_conn_params = &m_scan.conn_params;
    uint8_t                      con_cfg_tag   = m_scan.conn_cfg_tag;

    // Establish connection.
    ret_code_t err_code = sd_ble_gap_connect(p_addr, p_scan_params, p_conn_params, con_cfg_tag);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("sd_ble_gap_connect returns %d ", err_code);
        return false;
    }

    // 接続後に実行される関数の参照を退避
    resume_func_after_conn = _resume_function;
    return true;
}

bool ble_service_central_request_disconnection(void)
{
    // BLE接続がない場合は終了
    if (conn_handle == BLE_CONN_HANDLE_INVALID) {
        return true;
    }
    
    // BLE切断を切断
    ret_code_t err_code = sd_ble_gap_disconnect(conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    init_for_request_connection();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("sd_ble_gap_disconnect returns %d ", err_code);
        return false;
    }

    return true;
}

uint8_t *ble_service_central_connected_address(void)
{
    return bluetooth_addr.addr;
}

//
// BLE GAPイベント関連処理
//
void ble_service_central_gap_adv_report(ble_evt_t const *p_ble_evt)
{
    // 統計情報を更新
    ble_service_central_stat_adv_report(&p_ble_evt->evt.gap_evt.params.adv_report);
}

void ble_service_central_gap_connected(ble_evt_t const *p_ble_evt)
{
    if (ble_service_peripheral_mode()) {
        return;
    }

    // 接続中はハンドルを保持
    conn_handle = p_ble_evt->evt.gattc_evt.conn_handle;
    
    // 接続後の処理を継続する
    if (resume_func_after_conn != NULL) {
        // 接続完了後に実行される関数を実行
        (*resume_func_after_conn)(already_paired);
    }
}

void ble_service_central_gap_disconnected(ble_evt_t const *p_ble_evt)
{
    if (ble_service_peripheral_mode()) {
        return;
    }

    (void)p_ble_evt;
}

void ble_service_central_gap_evt_auth_status(ble_evt_t const *p_ble_evt)
{
    if (ble_service_peripheral_mode()) {
        return;
    }

    (void)p_ble_evt;
}

//
// Peer managerイベント関連処理
//
static void get_bluetooth_addr_connected(uint16_t peer_id)
{
    pm_peer_data_t         peer_data;
    pm_peer_data_bonding_t bond_data;

    peer_data.p_bonding_data = &bond_data;
    uint16_t buf_size = sizeof(bond_data);
    ret_code_t err_code = pds_peer_data_read(peer_id, PM_PEER_DATA_ID_BONDING, &peer_data, &buf_size);
    if (err_code == NRF_SUCCESS) {
        bluetooth_addr = bond_data.peer_ble_id.id_addr_info;
    } else {
        NRF_LOG_DEBUG("pds_peer_data_read returns 0x%04x", err_code);
    }
}

bool ble_service_central_pm_evt(void const *p_pm_evt)
{
    pm_evt_t const *p_evt = (pm_evt_t const *)p_pm_evt;
    switch (p_evt->evt_id) {
        case PM_EVT_BONDED_PEER_CONNECTED:
            // ペアリング済みである場合はフラグを設定
            NRF_LOG_DEBUG("Connected using bonding data");
            get_bluetooth_addr_connected(p_evt->peer_id);
            already_paired = true;
            break;

        default:
            break;
    }
    return false;
}
