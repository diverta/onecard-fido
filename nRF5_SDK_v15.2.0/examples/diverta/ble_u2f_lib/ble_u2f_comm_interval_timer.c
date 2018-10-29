#include "sdk_common.h"

#include "ble_u2f.h"
#include "ble_hci.h"
#include "app_timer.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_comm_interval_timer
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define COMMUNICATION_INTERVAL_MSEC 10000

// 無通信タイマー
APP_TIMER_DEF(m_ble_u2f_comm_interval_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

static void communication_interval_timeout_handler(void *p_context)
{
    if (p_context == NULL) {
        return;
    }

    // 直近のレスポンスから10秒を経過した場合、
    // nRF52から強制的にBLEコネクションを切断
    ble_u2f_t *p_u2f = (ble_u2f_t *)p_context;
    sd_ble_gap_disconnect(p_u2f->conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
}

static void communication_interval_timer_init(ble_u2f_t *p_u2f)
{
    if (app_timer_created == true) {
        return;
    }

    // 直近レスポンスからの経過秒数監視するためのタイマーを生成
    uint32_t err_code;
    err_code = app_timer_create(&m_ble_u2f_comm_interval_timer_id, APP_TIMER_MODE_SINGLE_SHOT, communication_interval_timeout_handler);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_create(m_ble_u2f_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_created = true;
}

void ble_u2f_comm_interval_timer_stop(ble_u2f_t *p_u2f)
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // 直近レスポンスからの経過秒数監視を停止
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_ble_u2f_comm_interval_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_ble_u2f_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
}

void ble_u2f_comm_interval_timer_start(ble_u2f_t *p_u2f)
{
    // タイマー生成
    if (app_timer_created == false) {
        communication_interval_timer_init(p_u2f);
        if (app_timer_created == false) {
            return;
        }
    }

    // タイマーが既にスタートしている場合は停止させる
    if (app_timer_started == true) {
        ble_u2f_comm_interval_timer_stop(p_u2f);
    }

    // 直近レスポンスからの経過秒数監視を開始
    uint32_t err_code = app_timer_start(m_ble_u2f_comm_interval_timer_id, APP_TIMER_TICKS(COMMUNICATION_INTERVAL_MSEC), p_u2f);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_ble_u2f_comm_interval_timer_id) returns %d ", err_code);
        return;
    }
    app_timer_started = true;
}
