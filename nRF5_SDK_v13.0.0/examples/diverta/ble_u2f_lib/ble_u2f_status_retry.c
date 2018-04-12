#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include "ble_u2f.h"
#include "ble_u2f_status.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_status_retry"
#include "nrf_log.h"

// タイマー
#include "app_timer.h"
#define RETRY_INTERVAL_MSEC 3000

APP_TIMER_DEF(m_ble_u2f_status_retry_timer_id);
static bool app_timer_created = false;
static bool app_timer_started = false;

// 再送エラーレスポンスのステータスワードを保持
static uint16_t m_err_status_word;
static uint8_t  data_buffer[2];

static void command_timer_handler(void *p_context)
{
    // コマンド、ステータスワードを格納
    data_buffer[0] = 0x9F;
    data_buffer[1] = (uint8_t)m_err_status_word;
    ble_u2f_status_setup(U2F_COMMAND_MSG, data_buffer, sizeof(data_buffer));

    // レスポンスを送信
    ble_u2f_t *p_u2f = (ble_u2f_t *)p_context;
    ble_u2f_status_response_send(p_u2f);
    NRF_LOG_DEBUG("ble_u2f_status_retry_error_response done: 0x%02x%02x \r\n",
        data_buffer[0], data_buffer[1]);
}

static void ble_u2f_status_retry_init()
{
    uint32_t err_code;
    if (app_timer_created == false) {
        err_code = app_timer_create(&m_ble_u2f_status_retry_timer_id, APP_TIMER_MODE_SINGLE_SHOT, command_timer_handler);
        if (err_code != NRF_SUCCESS) {
            NRF_LOG_ERROR("app_timer_create(m_ble_u2f_status_retry_timer_id) returns %d \r\n", err_code);
            return;
        }
        app_timer_created = true;
    }
}

static void ble_u2f_status_retry_terminate()
{
    if (app_timer_created == false || app_timer_started == false) {
        return;
    }

    // タイマーを停止する
    app_timer_started = false;
    uint32_t err_code = app_timer_stop(m_ble_u2f_status_retry_timer_id);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_stop(m_ble_u2f_status_retry_timer_id) returns %d \r\n", err_code);
        return;
    }
}

static void ble_u2f_status_retry_start(ble_u2f_t *p_u2f)
{
    if (app_timer_created == false) {
        return;
    }

    // タイマーが既にスタートしている場合は停止させる
    if (app_timer_started == true) {
        ble_u2f_status_retry_terminate();
    }

    // タイマーを開始する
    uint32_t err_code = app_timer_start(m_ble_u2f_status_retry_timer_id, APP_TIMER_TICKS(RETRY_INTERVAL_MSEC), p_u2f);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_timer_start(m_ble_u2f_status_retry_timer_id) returns %d \r\n", err_code);
        return;
    }
    app_timer_started = true;
}

void ble_u2f_status_retry_error_response(ble_u2f_t *p_u2f, uint16_t err_status_word)
{
    // タイマーが生成されていない場合は生成
    ble_u2f_status_retry_init();

    // タイマーを開始する
    ble_u2f_status_retry_start(p_u2f);
    
    // ステータスワードを保持
    m_err_status_word = err_status_word;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
