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

// for processing & lighting LED on/off
#include "fido_board.h"

// for keepalive timer
#include "fido_timer.h"

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

#define LONG_PUSH_TIMEOUT               3000

//
// ボタン長押し検知関連
//
static bool m_long_pushed = false;
static bool m_push_initial = true;

static void on_button_evt(uint8_t pin_no, uint8_t button_action)
{
	switch (button_action) {
	case APP_BUTTON_ACTION_PUSH:
        if (m_push_initial) {
            m_push_initial = false;
        }
        if (pin_no == PIN_MAIN_SW_IN) {
            fido_button_long_push_timer_start(LONG_PUSH_TIMEOUT, NULL);
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
            fido_button_long_push_timer_stop();
            
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

void fido_command_long_push_timer_handler(void *p_context)
{
    (void)p_context;

	m_long_pushed = true;
	on_button_evt(PIN_MAIN_SW_IN, APP_BUTTON_ACTION_LONG_PUSH);
}

//
// タイマーを追加
//
void fido_button_timers_init(void)
{
    // ボタン長押し検知用タイマー
    fido_button_long_push_timer_init();
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

void fido_command_keepalive_timer_handler(void *p_context)
{
    // キープアライブ・コマンドを実行する
    if (p_context == NULL) {
        fido_ctap2_command_keepalive_timer_handler();
    } else {
        ble_u2f_command_keepalive_timer_handler(p_context);
    }
}

void fido_user_presence_terminate(void)
{
    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
}

void fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context)
{
    // タイマーが生成されていない場合は生成する
    fido_keepalive_interval_timer_start(timeout_msec, p_context);

    // LED点滅を開始
    fido_processing_led_on(LED_FOR_USER_PRESENCE, LED_ON_OFF_INTERVAL_MSEC);
}

uint8_t fido_user_presence_verify_end(void)
{
    // LEDを消灯させる
    fido_processing_led_off();

    // タイマーを停止する
    fido_keepalive_interval_timer_stop();
    
    // User presence byte(0x01)を生成
    return 0x01;
}
