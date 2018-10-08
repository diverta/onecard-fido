/* 
 * File:   one_card_main.c
 * Author: makmorit
 *
 * Created on 2018/10/08, 10:37
 */

//
// One Card固有の定義
//
#define NRF_BLE_GATT_MAX_MTU_SIZE 67

#include "nrf_ble_gatt.h"
#include "ble_srv_common.h"
#include "ble_dis.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "peer_manager_types.h"
#include "ble_conn_state.h"

#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "app_button.h"

// for logging informations
//#define NRF_LOG_MODULE_NAME "one_card_main"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// for FIDO
#include "ble_u2f.h"

//
// アドバタイズ用設定
//   APP_ADV_INTERVAL 0.625ms*300=187.5ms
//   APP_ADV_TIMEOUT_IN_SECONDS 180sec
//
#define APP_ADV_INTERVAL                300
#define APP_ADV_TIMEOUT_IN_SECONDS      180

#define PIN_MAIN_SW_IN                  BSP_BUTTON_0

#define APP_BUTTON_NUM                  2
#define APP_BUTTON_DELAY                APP_TIMER_TICKS(100)
#define APP_BUTTON_ACTION_PUSH          APP_BUTTON_PUSH
#define APP_BUTTON_ACTION_RELEASE       APP_BUTTON_RELEASE
#define APP_BUTTON_ACTION_LONG_PUSH     2

#define LONG_PUSH_TIMEOUT               APP_TIMER_TICKS(3000)

//
// ボタン長押し検知関連
//
APP_TIMER_DEF(m_long_push_timer_id);
static bool m_long_pushed = false;

static void on_button_evt(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

	switch (button_action) {
	case APP_BUTTON_ACTION_PUSH:
        if (pin_no == PIN_MAIN_SW_IN) {
            err_code = app_timer_start(m_long_push_timer_id, LONG_PUSH_TIMEOUT, NULL);
            APP_ERROR_CHECK(err_code);
        }
		break;
		
	case APP_BUTTON_ACTION_RELEASE:
        if (m_long_pushed) {
            m_long_pushed = false;
            break;
        }

        if (pin_no == PIN_MAIN_SW_IN) {
            app_timer_stop(m_long_push_timer_id);
            //
            // TODO: FIDO U2F固有の処理を実行
            //
            NRF_LOG_INFO("PIN_MAIN_SW_IN short pushed(%d).", pin_no);
        }
		break;
		
	case APP_BUTTON_ACTION_LONG_PUSH:
        if (pin_no == PIN_MAIN_SW_IN) {
            // 
            // TODO: FIDO U2F固有の処理を実行
            //
            NRF_LOG_INFO("PIN_MAIN_SW_IN long pushed(%d).", pin_no);
        }
		break;
		
	default:
		break;
	}
}

static void on_long_push_timer_timeout(void *p_context)
{
	NRF_LOG_INFO("Button long pushed.");

	m_long_pushed = true;
	on_button_evt(PIN_MAIN_SW_IN, APP_BUTTON_ACTION_LONG_PUSH);
}

//
// タイマーを追加
//
void one_card_timers_init(void)
{
    ret_code_t err_code;
    
    // ボタン長押し検知用タイマー（３秒）
    err_code = app_timer_create(&m_long_push_timer_id, 
        APP_TIMER_MODE_SINGLE_SHOT, on_long_push_timer_timeout);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_INFO("app_timer_create() (%d).", err_code);
}

static const app_button_cfg_t m_app_buttons[APP_BUTTON_NUM] = {
    {PIN_MAIN_SW_IN, false, BUTTON_PULL, on_button_evt}
};

//
// ボタンをカスタマイズ
//
void one_card_buttons_init(void)
{
    ret_code_t err_code;

    err_code = app_button_init((app_button_cfg_t*)m_app_buttons, APP_BUTTON_NUM, APP_BUTTON_DELAY);
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);

    // 
    // TODO:
    // BLE U2Fで使用するLEDのピン番号を設定
    //m_u2f.led_for_processing_fido = PIN_LED1;
    //m_u2f.led_for_pairing_mode    = PIN_LED2;
    //m_u2f.led_for_user_presence   = PIN_LED3;
    // 
}

//
// BLEサービス初期化
//
void one_card_services_init(void)
{
    // TODO: U2Fサービスを初期化
}
