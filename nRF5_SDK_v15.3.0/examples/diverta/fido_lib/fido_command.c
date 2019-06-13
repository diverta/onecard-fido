/* 
 * File:   fido_command.c
 * Author: makmorit
 *
 * Created on 2019/02/11, 11:31
 */
#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "app_button.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_command
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for FIDO
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_ctap2_command.h"
#include "hid_u2f_command.h"
#include "fido_ctap2_command.h"
#include "hid_fido_command.h"
#include "nfc_fido_command.h"
#include "fido_ble_main.h"

// for processing LED on/off
#include "fido_processing_led.h"

// for lighting LED on/off
#include "fido_idling_led.h"

//
// ボタンのピン番号
//
#define PIN_MAIN_SW_IN                  BUTTON_1
#define PIN_MAIN_SW_PULL                BUTTON_PULL

#define APP_BUTTON_NUM                  1
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
static bool m_push_initial = true;

static void on_button_evt(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

	switch (button_action) {
	case APP_BUTTON_ACTION_PUSH:
        if (m_push_initial) {
            m_push_initial = false;
        }
        if (pin_no == PIN_MAIN_SW_IN) {
            err_code = app_timer_start(m_long_push_timer_id, LONG_PUSH_TIMEOUT, NULL);
            APP_ERROR_CHECK(err_code);
        }
		break;
		
	case APP_BUTTON_ACTION_RELEASE:
        if (m_push_initial) {
            // ボタン長押し中にリセット後、
            // 単独でこのイベントが発生した場合は無視
            m_push_initial = false;
            break;
        }
        if (m_long_pushed) {
            m_long_pushed = false;
            break;
        }

        if (pin_no == PIN_MAIN_SW_IN) {
            app_timer_stop(m_long_push_timer_id);
            
            // FIDO U2F固有の処理を実行
            if (ble_u2f_command_on_mainsw_event(fido_ble_get_U2F_context()) == true) {
                break;
            }
            if (hid_u2f_command_on_mainsw_event() == true) {
                break;
            }
            fido_ctap2_command_on_mainsw_event();
            ble_ctap2_command_on_mainsw_event();
        }
		break;
		
	case APP_BUTTON_ACTION_LONG_PUSH:
        if (pin_no == PIN_MAIN_SW_IN) {
            // FIDO U2F固有の処理を実行
            if (ble_u2f_command_on_mainsw_long_push_event(fido_ble_get_U2F_context()) == true) {
                break;
            }
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
void fido_button_timers_init(void)
{
    ret_code_t err_code;
    
    // ボタン長押し検知用タイマー（３秒）
    err_code = app_timer_create(&m_long_push_timer_id, 
        APP_TIMER_MODE_SINGLE_SHOT, on_long_push_timer_timeout);
    APP_ERROR_CHECK(err_code);
}

static const app_button_cfg_t m_app_buttons[APP_BUTTON_NUM] = {
    {PIN_MAIN_SW_IN, false, PIN_MAIN_SW_PULL, on_button_evt}
};

//
// ボタンをカスタマイズ
//
void fido_button_init(void)
{
    ret_code_t err_code;

    err_code = app_button_init((app_button_cfg_t*)m_app_buttons, APP_BUTTON_NUM, APP_BUTTON_DELAY);
    if (err_code) {
        NRF_LOG_ERROR("app_button_init returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);

    err_code = app_button_enable();
    if (err_code) {
        NRF_LOG_ERROR("app_button_enable returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);
}

static void fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // FDS処理完了後のBLE処理を実行
    ble_u2f_command_on_fs_evt(p_evt);

    // FDS処理完了後のUSB HID処理を実行
    hid_fido_command_on_fs_evt(p_evt);

    // FDS処理完了後のNFC処理を実行
    nfc_fido_command_on_fs_evt(p_evt);
}

void fido_command_fds_register(void)
{
    // FDS処理完了後の処理をFDSに登録
    ret_code_t err_code = fds_register(fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}

void fido_command_on_process_timedout(void) 
{
    // 処理タイムアウト発生時の処理を実行
    //
    // 処理中表示LEDが点滅していた場合は
    // ここでLEDを消灯させる
    fido_processing_led_off();

    // アイドル時点滅処理を再開
    fido_idling_led_on();
}
