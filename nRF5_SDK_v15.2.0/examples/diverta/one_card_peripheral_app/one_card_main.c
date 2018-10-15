/* 
 * File:   one_card_main.c
 * Author: makmorit
 *
 * Created on 2018/10/08, 10:37
 */

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
#define NRF_LOG_MODULE_NAME one_card_main
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

// for FIDO
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_init.h"

//
// U2F関連の共有情報
//
static ble_u2f_t m_u2f;

//
// アドバタイズ用設定
//   APP_ADV_INTERVAL 0.625ms*300=187.5ms
//   APP_ADV_TIMEOUT_IN_SECONDS 180sec
//
#define APP_ADV_INTERVAL                300
#define APP_ADV_TIMEOUT_IN_SECONDS      180

//
// ボタン、LEDのピン番号
//
#define PIN_MAIN_SW_IN                  BUTTON_1
#define PIN_MAIN_SW_PULL                BUTTON_PULL
#define PIN_LED1                        LED_1
#define PIN_LED2                        LED_2
#define PIN_LED3                        LED_3

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
            if (ble_u2f_command_on_mainsw_event(&m_u2f) == true) {
                break;
            }
        }
		break;
		
	case APP_BUTTON_ACTION_LONG_PUSH:
        if (pin_no == PIN_MAIN_SW_IN) {
            // FIDO U2F固有の処理を実行
            if (ble_u2f_command_on_mainsw_long_push_event(&m_u2f) == true) {
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
    {PIN_MAIN_SW_IN, false, PIN_MAIN_SW_PULL, on_button_evt}
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

    // BLE U2Fで使用するLEDのピン番号を設定
    m_u2f.led_for_processing_fido = PIN_LED1;
    m_u2f.led_for_pairing_mode    = PIN_LED2;
    m_u2f.led_for_user_presence   = PIN_LED3;
}

//
// BLEスタック初期化
//
void one_card_ble_stack_init(uint8_t conn_cfg_tag, uint32_t p_ram_start)
{
    ret_code_t err_code;
    ble_cfg_t  ble_cfg;

    // FIDO機能に対応できるようにするため、
    // デフォルトのBLE設定を変更する
    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, p_ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum event length.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = conn_cfg_tag;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = 320;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_GAP_CONN_COUNT_DEFAULT;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, p_ram_start);
    APP_ERROR_CHECK(err_code);
}

void one_card_gatt_init(nrf_ble_gatt_t *p_gatt)
{
    // TODO: U2F固有の処理があれば追加
}

void one_card_advertising_init(ble_advertising_init_t *p_init)
{
    // アドバタイジング設定の前に、
    // ペアリングモードをFDSから取得
    ble_u2f_pairing_get_mode(&m_u2f);
    
    // ペアリングモードでない場合は、
    // ディスカバリーができないよう設定
    p_init->advdata.flags = ble_u2f_pairing_advertising_flag();
}

//
// BLEサービス初期化
//
void one_card_services_init(void)
{
    // U2Fサービスを初期化
    ble_u2f_init_services(&m_u2f);
}

void one_card_peer_manager_init(void)
{
    // FDS処理完了後のU2F処理を続行させる
    ret_code_t err_code = fds_register(ble_u2f_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}

ble_u2f_t *one_card_get_U2F_context(void)
{
    // U2F関連の共有情報
    return &m_u2f;
}
