/* 
 * File:   fido_board.c
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#include "sdk_common.h"

#include "bsp_btn_ble.h"
#include "app_timer.h"
#include "app_button.h"

// for lighting LED
#include "nrf_gpio.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_board
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_board.h"
#include "fido_command.h"
#include "fido_timer.h"

// for fido_ble_pairing_change_mode
#include "ble_service_peripheral.h"
#include "fido_ble_pairing.h"

#include "fido_platform.h"

//
// LEDのピン
//
#if defined(BOARD_PCA10059)
#define LED_R   NRF_GPIO_PIN_MAP(1, 10)
#define LED_Y   NRF_GPIO_PIN_MAP(0, 26)
#else
#define LED_R   LED_1
#define LED_Y   LED_2
#endif

//
// ボタンのピン番号
//
#define PIN_MAIN_SW_IN                  BUTTON_1
#define PIN_MAIN_SW_PULL                BUTTON_PULL

#define APP_BUTTON_NUM                  1
#define APP_BUTTON_DELAY                APP_TIMER_TICKS(100)
#define APP_BUTTON_ACTION_PUSH          APP_BUTTON_PUSH
#define APP_BUTTON_ACTION_RELEASE       APP_BUTTON_RELEASE

#define LONG_PUSH_TIMEOUT               3000

//
// デモ機能
//
#define DEMO_FUNCTION_ENABLE false
#if DEMO_FUNCTION_ENABLE
#include "ble_service_central_demo.h"
#endif

//
// ボタン定義
//
static void on_button_evt(uint8_t pin_no, uint8_t button_action);
static const app_button_cfg_t m_app_buttons[APP_BUTTON_NUM] = {
    {PIN_MAIN_SW_IN, false, PIN_MAIN_SW_PULL, on_button_evt}
};

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
            // ボタンが長押しされた時の処理を実行
            if (ble_service_peripheral_mode()) {
                // BLEペリフェラルが稼働時は、
                // ペアリングモード変更を実行
                fido_ble_pairing_change_mode();
            }
            break;
        }

        if (pin_no == PIN_MAIN_SW_IN) {
            fido_button_long_push_timer_stop();
            
            // FIDO固有の処理を実行
            fido_command_mainsw_event_handler();

#if DEMO_FUNCTION_ENABLE
            if (ble_service_peripheral_mode() == false) {
                // デモ機能を実行
                ble_service_central_demo_button_pressed();
            }
#endif
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
    
    if (ble_service_peripheral_mode()) {
        // ペアリングモードに遷移させるための長押しの場合、
        // このタイミングで、黄色LEDを連続点灯させる
        fido_status_indicator_pairing_mode();
    }
}

//
// タイマーを追加
//
void fido_button_timers_init(void)
{
    // ボタン長押し検知用タイマー
    fido_button_long_push_timer_init();
}

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

//
// LED関連
//
void led_light_pin_set(LED_COLOR led_color, bool led_on)
{
    // FIDO機能で使用するLEDのピン番号を設定
    // nRF52840 Dongleでは以下の割り当てになります。
    //   LED2=Red
    //   LED3=Green
    //   LED4=Blue
    uint32_t pin_number;
    switch (led_color) {
        case LED_COLOR_RED:
            pin_number = LED_2;
            break;
        case LED_COLOR_GREEN:
            pin_number = LED_3;
            break;
        case LED_COLOR_BLUE:
            pin_number = LED_4;
            break;
        case LED_COLOR_BUSY:
            pin_number = LED_R;
            break;
        case LED_COLOR_PAIR:
            pin_number = LED_Y;
            break;
        default:
            return;
    }
    
    // LEDを出力設定
    nrf_gpio_cfg_output(pin_number);
    if (led_on) {
        // LEDを点灯させる
        nrf_gpio_pin_clear(pin_number);
    } else {
        // LEDを消灯させる
        nrf_gpio_pin_set(pin_number);
    }
}

//
// デバイスのバージョン情報関連
// 
bool fido_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size)
{
    // 格納領域を初期化
    memset(info_csv_data, 0, *info_csv_size);

    // 各項目をCSV化し、引数のバッファに格納
    sprintf((char *)info_csv_data, 
        "DEVICE_NAME=\"%s\",FW_REV=\"%s\",HW_REV=\"%s\"", DEVICE_NAME, FW_REV, HW_REV);
    *info_csv_size = strlen((char *)info_csv_data);
    NRF_LOG_DEBUG("Application version info csv created (%d bytes)", *info_csv_size);
    return true;
}
