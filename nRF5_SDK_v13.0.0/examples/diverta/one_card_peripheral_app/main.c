/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */


/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "fstorage.h"
#include "fds.h"
#include "peer_manager.h"

#include "sensorsim.h"
#include "nrf_gpio.h"

#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include "../magstripe_lib/MagStripeData.h"
#include "one_card_config.h"
#include "ble_one_card.h"
#include "card_manager.h"
#include "one_card_spi_master.h"

/*
 * For FIDO BLE Service.
 */
#include "ble_u2f.h"
#include "ble_u2f_status.h"
#include "ble_u2f_init.h"
#include "ble_u2f_command.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_pairing_lesc.h"
#include "ble_u2f_util.h"
#include "ble_dis.h"
#include "ble_u2f_flash.h"

/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

#define DEVICE_NAME                     "OneCard_Peripheral"                        /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "Diverta,Inc"                               /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                         /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout in units of seconds. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                      /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  1                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define PASSKEY_LENGTH                  6                                           /**< Length of pass-key received by the stack for display. */

#define CONN_CFG_TAG                    1

/*
 * timer.
 */
#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define NO_OPERATION_TIME_OUT_SEC       30                                          /**< No operation timer. */
#define NO_OPERATION_TIMER_INTERVAL     APP_TIMER_TICKS(1000)
#define TRIGGER_DISABLED_TIME_OUT_SEC   1                                           /**< Continuous trigger disabled timer. */
#define TRIGGER_DISABLED_TIMER_INTERVAL APP_TIMER_TICKS(1000)
#define CARD_SYNC_TIME_OUT_SEC          1                                           /**< Card sync timer. */
#define CARD_SYNC_TIMER_INTERVAL        APP_TIMER_TICKS(5000)
#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(400)                        /**< Delay after connection until Security Request is sent, if necessary (ticks). */
#define LONG_PUSH_TIMEOUT               APP_TIMER_TICKS(3000)                       /**< The time to hold for a long push (in milliseconds). */

/*
 * button.
 */
#define APP_BUTTON_NUM                  2
#define APP_BUTTON_DELAY                APP_TIMER_TICKS(100)
#define APP_BUTTON_ACTION_PUSH          APP_BUTTON_PUSH                             /**< Represents pushing a button. */
#define APP_BUTTON_ACTION_RELEASE       APP_BUTTON_RELEASE                          /**< Represents releasing a button. */
#define APP_BUTTON_ACTION_LONG_PUSH     2                                           /**< Represents pushing and holding a button. */

/*
 * ble : service
 *     # Device Information Service.
 */
#define MANUFACTURER_NAME				"Diverta,Inc"								/**< Manufacturer. Will be passed to Device Information Service. */
#define MODEL_NUM						"One Card Example"							/**< Model number. Will be passed to Device Information Service. */
#define FW_REV							"13.0.0"									/**< firmware revision. Will be passed to Device Information Service. */

/*
 * ble : service
 *     # Diverta OneCard Service.
 */
#define SERIAL_CODE_BASE				"12345678"
#define SERIAL_CODE_BLANK				"000000000000"

#define SERIAL_CODE_LEN					BLE_ONE_CARD_SERIAL_CODE_LEN

/*
 * MagStripeData Library.
 */
#define MAGSTRIPEDATA_SYNC_BIT_LENGTH		8											//!< 8 or 100?
#define MAGSTRIPEDATA_F2F_PULSE_WIDTH_US	100											//!< F2F Pulse width(us).
#define MAGSTRIPEDATA_TRIGGER_DELAY_MS		50											//!< Trigger delay.


/*******************************************************************************
 * function prototype.
 ******************************************************************************/
/*
 * log.
 */
static void log_init(void);

/*
 * timer.
 */
static void timers_init(void);
static void timers_start(void);
static void on_no_operation_timer_timeout(void *p_context);
static void on_trigger_disabled_timer_timeout(void *p_context);
static void on_card_sync_timer_timeout(void *p_context);
static void on_seq_req_timer_timeout(void *p_context);

/*
 * gpiote.
 */
static void gpiote_init(void);
static void gpiote_power_cont(void);
static void gpiote_drive_trigger(void);

/*
 * button.
 */
static void buttons_init(void);
static void on_button_evt(uint8_t pin_no, uint8_t button_action);

/*
 * peer manager.
 */
static void peer_manager_init(void);
static void on_pm_evt(pm_evt_t const * p_evt);

/*
 * ble.
 */
static void ble_stack_init(void);
static void ble_gap_init(void);
static void ble_gatt_init(void);
static void ble_services_init(void);
static void ble_conn_init(void);
static void ble_adv_init(void);
static void ble_adv_start(void);
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
static void on_conn_params_error_handler(uint32_t nrf_error);
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void on_ble_evt(ble_evt_t * p_ble_evt);
static void on_one_card_evt(uint16_t evt_id);

/*
 * power save.
 */
static void power_manage(void);
static void sleep_mode_enter(void);

/*
 * MagStripeData.
 */
static void magstripedata_init(void);
static void magstripedata_switch_card_selection(void);
static void magstripedata_transfer(uint8_t card_type, const uint8_t *p_card_code, uint16_t card_code_len);
static bool magstripedata_is_available_card(void);

/*
 * Serial Code.
 */
static void serial_code_generate(void);
static uint8_t serial_code_compare(const uint8_t *p_expected, const uint8_t *p_actual);


/*******************************************************************************
 * static fields.
 ******************************************************************************/
APP_TIMER_DEF(m_no_operation_timer_id);                                             /**< No Operation timer. */
APP_TIMER_DEF(m_trigger_disabled_timer_id);                                         /**< Trigger Disabled timer. */
APP_TIMER_DEF(m_card_sync_timer_id);                                                /**< Card sync timer. */
APP_TIMER_DEF(m_sec_req_timer_id);                                                  /**< Security Request timer. */
APP_TIMER_DEF(m_long_push_timer_id);                                                /**< Button long push timer. */

static int m_no_operation_timer_sec = 0;
static int m_trigger_disabled_timer_sec = 0;
static int m_card_sync_timer_sec = 0;

static const app_button_cfg_t m_app_buttons[APP_BUTTON_NUM] = {
	{PIN_SCAN_SW_IN, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLDOWN, on_button_evt},
	{PIN_MAIN_SW_IN, APP_BUTTON_ACTIVE_LOW , NRF_GPIO_PIN_PULLUP  , on_button_evt}
};

static bool m_long_pushed = false;

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                            /**< Handle of the current connection. */
static nrf_ble_gatt_t m_gatt;                                                       /**< GATT module instance. */

// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {
	{BLE_UUID_U2F_SERVICE,                BLE_UUID_TYPE_BLE},
	{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
	{BLE_UUID_ONE_CARD_SERVICE          , BLE_UUID_TYPE_VENDOR_BEGIN}
};                                                                                  /**< Universally unique service identifiers. */

/* YOUR_JOB: Declare all services structure your application is using
   static ble_xx_service_t                     m_xxs;
   static ble_yy_service_t                     m_yys;
 */
static const ble_one_card_t *m_ble_one_card = NULL;                                 /**< One Card Service. */

static uint8_t m_serial_code[SERIAL_CODE_LEN + 1];
static uint8_t m_serial_retry_count = 0;

/*
 * For FIDO BLE Service
 */
static ble_u2f_t m_u2f;


/*******************************************************************************
 * entry point.
 ******************************************************************************/
/*
 * @brief Function for application main entry.
 */
int main(void)
{
    log_init();
    NRF_LOG_INFO("[APP]Launched.\r\n");

    // initialize application.
    timers_init();
    gpiote_init();
    buttons_init();
    NRF_LOG_INFO("[APP]Application initialized.\r\n");

    // initialize ble stack.
    ble_stack_init();
    NRF_LOG_INFO("[APP]BLE stack initialized.\r\n");

    // initialize ble services.
    ble_gap_init();
    ble_gatt_init();
    ble_conn_init();
    ble_services_init();
    NRF_LOG_INFO("[APP]BLE services initialized.\r\n");

    // initialize peer manager.
    peer_manager_init();
    NRF_LOG_INFO("[APP]Peer manager initialized.\r\n");
    
    // アドバタイジング設定の前に、
    // ペアリングモードをFDSから取得
    ble_u2f_pairing_get_mode(&m_u2f);
    ble_adv_init();
    NRF_LOG_INFO("[APP]BLE connection initialized.\r\n");

    // Magstripe Data.
    magstripedata_init();
    one_card_spi_master_init();

    // Serial Code.
    serial_code_generate();
    ble_one_card_set_serial_code(m_ble_one_card, (uint8_t*)SERIAL_CODE_BLANK, SERIAL_CODE_LEN);

// start execution.
    timers_start();
    ble_adv_start();
    NRF_LOG_INFO("[APP]BLE advertising started.\r\n");

    // Enter main loop.
    for (;;) {
        if (NRF_LOG_PROCESS() == false) {
            power_manage();
        }
    }
}

/*******************************************************************************
 * callback handler.
 ******************************************************************************/
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

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void on_sys_evt_dispatch(uint32_t sys_evt)
{
	//NRF_LOG_INFO("[APP]on_sys_evt_dispatch(%u).\r\n", (unsigned int)sys_evt);

    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}

/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    NRF_LOG_DEBUG("on_ble_evt_dispatch called (evt_id=0x%02x) \r\n", p_ble_evt->header.evt_id);

    // ペアリングモードでない場合は、
    // ペアリング要求に応じないようにする
    if (ble_u2f_pairing_reject_request(&m_u2f, p_ble_evt) == true) {
        return;
    }

    ble_conn_state_on_ble_evt(p_ble_evt);
    pm_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    nrf_ble_gatt_on_ble_evt(&m_gatt, p_ble_evt);

    if (ble_u2f_on_ble_evt(&m_u2f, p_ble_evt) == false) {
        // FIDO U2Fで処理されたイベントは、
        // One Cardサービスで重複処理されないようにする
        ble_one_card_on_ble_evt(p_ble_evt);
    }

    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void on_pm_evt(pm_evt_t const * p_evt)
{
    // ペアリング済みである端末からの
    // 再ペアリング要求を受入れるようにする
    if (ble_u2f_pairing_allow_repairing(p_evt) == true) {
        return;
    }
    // ペアリング情報の削除が完了したときの処理を行う
    if (ble_u2f_pairing_delete_bonds_response(p_evt) == true) {
        return;
    }
    // ペアリングが無効になってしまった場合
    // ペアリングモードLED点滅を開始させる
    ble_u2f_pairing_notify_unavailable(&m_u2f, p_evt);

    switch (p_evt->evt_id) {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("[APP]Connected to a previously bonded device.\r\n");
        }
		break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("[APP]Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.\r\n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        }
		break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
            NRF_LOG_ERROR("[APP]Secured connection failed. \r\n");
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        }
		break;

        case PM_EVT_STORAGE_FULL:
        {
            // FDS GCを実行
            ble_u2f_flash_force_fdc_gc();
        }
		break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        }
		break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        }
		break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } 
		break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        }
		break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        }
		break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void on_conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    //NRF_LOG_INFO("[APP]on_adv_evt(%d).\r\n", ble_adv_evt);
    
    switch (ble_adv_evt) {
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        case BLE_ADV_EVT_FAST:
            break;

        default:
            break;
    }
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    // LESCペアリングによる鍵交換処理
    if (ble_u2f_pairing_lesc_on_ble_evt(&m_u2f, p_ble_evt) == true) {
        return;
    }

    ret_code_t err_code = NRF_SUCCESS;

    //NRF_LOG_INFO("[APP]on_ble_evt(0x%02x).\r\n", p_ble_evt->header.evt_id);
    
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("[APP]Disconnected.\r\n");
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break; // BLE_GAP_EVT_DISCONNECTED

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("[APP]Connected.\r\n");
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break; // BLE_GAP_EVT_CONNECTED

        case BLE_GAP_EVT_DATA_LENGTH_UPDATE_REQUEST:
        {
            ble_gap_data_length_params_t dl_params;

            // Clearing the struct will effectivly set members to @ref BLE_GAP_DATA_LENGTH_AUTO
            memset(&dl_params, 0, sizeof(ble_gap_data_length_params_t));
            err_code = sd_ble_gap_data_length_update(p_ble_evt->evt.gap_evt.conn_handle, &dl_params, NULL);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_SYS_ATTR_MISSING

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("[APP]GATT Client Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTC_EVT_TIMEOUT

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("[APP]GATT Server Timeout.\r\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break; // BLE_GATTS_EVT_TIMEOUT

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gattc_evt.conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break; // BLE_EVT_USER_MEM_REQUEST

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            ble_u2f_status_on_tx_complete(&m_u2f);
            break; // BLE_GATTS_EVT_HVN_TX_COMPLETE

        default:
            // No implementation needed.
            break;
    }
}

/**
 * @brief One Card Service イベントハンドラ
 *
 * @param[in]	evt_id	イベントID.
 */
static void on_one_card_evt(uint16_t evt_id)
{
	NRF_LOG_INFO("[APP]on_one_card_evt(%d).\r\n", evt_id);
	
	switch (evt_id) {
	case BLE_ONE_CARD_EVT_SYNC_CARD_NO_WRITE:
		{
			uint8_t sync_card_no;
			sync_card_no = ble_one_card_sync_card_no(m_ble_one_card);
			NRF_LOG_INFO("[APP]Sync Card Number : %#x.\r\n", sync_card_no);
			
			if (sync_card_no <= CARD_LIST_NUM - 1) {
				card_manager_set_sync_card_no(sync_card_no);
			}
			else {
				NRF_LOG_WARNING("[APP]Invalid Card Number!!\r\n");
			}
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_CARD_CODE_WRITE:
		{
			const uint8_t	*p_card_code_str = NULL;
			uint16_t		card_code_len = 0;
			ble_one_card_card_code(m_ble_one_card, &p_card_code_str, &card_code_len);
			NRF_LOG_INFO("[APP]Card Code : %s.\r\n", (uint32_t)p_card_code_str);

			card_manager_set_card_code(p_card_code_str, card_code_len);
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_CARD_TYPE_WRITE:
		{
			uint8_t card_type;
			card_type = ble_one_card_card_type(m_ble_one_card);
			NRF_LOG_INFO("[APP]Card Type : %#x.\r\n", card_type);

			if ((ONECARD_TRACK_1 <= card_type) && (card_type <= ONECARD_JIS_2)) {
				card_manager_set_card_type(card_type);
			}
			else {
				NRF_LOG_WARNING("[APP]Invalid Card Type!!\r\n");
			}
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_CARD_TITLE_WRITE:
		{
			const uint8_t	*p_card_title_str = NULL;
			uint16_t		card_title_len = 0;
			ble_one_card_card_title(m_ble_one_card, &p_card_title_str, &card_title_len);
			NRF_LOG_INFO("[APP]Card Title : %s.\r\n", (uint32_t)p_card_title_str);

			card_manager_set_card_title(p_card_title_str, card_title_len);
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_CARD_SUB_TITLE_WRITE:
		{
			const uint8_t	*p_card_sub_title_str = NULL;
			uint16_t		card_sub_title_len = 0;
			ble_one_card_card_sub_title(m_ble_one_card, &p_card_sub_title_str, &card_sub_title_len);
			NRF_LOG_INFO("[APP]Card Sub Title : %s.\r\n", (uint32_t)p_card_sub_title_str);
			
			card_manager_set_card_subtitle(p_card_sub_title_str, card_sub_title_len);
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_LOCK_TIME_WRITE:
		{
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_SERIAL_CODE_WRITE:
		{
			const uint8_t *p_card_serial_code_str = NULL;
			uint16_t card_serial_code_len = 0;
			ble_one_card_serial_code(m_ble_one_card, &p_card_serial_code_str, &card_serial_code_len);
			NRF_LOG_INFO("[APP]Serial Code : %s.\r\n", (uint32_t)p_card_serial_code_str);
			
			// verify serial code.
			if (serial_code_compare(p_card_serial_code_str, (const uint8_t*)m_serial_code) == 0) {
				NRF_LOG_INFO("[APP]Verify Serial Code.\r\n");
			}
			else {
				NRF_LOG_WARNING("[APP]Invalid Serial Code.\r\n");
				
				// increment the retry count.
				m_serial_retry_count++;
				
				// retry count is over?
				if (2 < m_serial_retry_count) {
					// TODO
				}
			}
			
			// cleanup.
			ble_one_card_set_serial_code(m_ble_one_card, (uint8_t*)SERIAL_CODE_BLANK, SERIAL_CODE_LEN);
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case BLE_ONE_CARD_EVT_RX_WRITE:
		{
			const uint8_t *p_rx_str = NULL;
			uint16_t rx_len = 0;
			ble_one_card_rx(m_ble_one_card, &p_rx_str, &rx_len);
			NRF_LOG_INFO("[APP]Rx : %s.\r\n", (uint32_t)p_rx_str);
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		
	default:
		break;
	}
}


/**
 * @brief Timer event handler.
 *
 * @param[in]   p_context
 */
static void on_no_operation_timer_timeout(void *p_context)
{
	gpiote_power_cont();

	// count down.
	if (0 < m_no_operation_timer_sec) {
		m_no_operation_timer_sec--;
		//NRF_LOG_INFO("[APP]No operation timer timeout(%d).\r\n", m_no_operation_timer_sec);
		// time up.
		if (m_no_operation_timer_sec == 0) {
			// TODO
		}
	}
}

/**
 * @brief Timer event handler.
 *
 * @param[in]   p_context
 */
static void on_trigger_disabled_timer_timeout(void *p_context)
{
    // count down.
    if (0 < m_trigger_disabled_timer_sec) {
        m_trigger_disabled_timer_sec--;
        NRF_LOG_INFO("[APP]Trigger disabled timer timeout(%d).\r\n", m_trigger_disabled_timer_sec);
    }
}

/**
 * @brief Timer event handler.
 *
 * @param[in]   p_context
 */
static void on_card_sync_timer_timeout(void *p_context)
{
	// count down.
	if (0 < m_card_sync_timer_sec) {
		m_card_sync_timer_sec--;
		NRF_LOG_INFO("[APP]Card sync timer timeout(%d).\r\n", m_card_sync_timer_sec);
		// time up.
		if (m_card_sync_timer_sec == 0) {
			// TODO
		}
	}
}

/**
 * @brief Timer event handler.
 *
 * @param[in]   p_context
 */
static void on_seq_req_timer_timeout(void *p_context)
{
    ret_code_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
        // Initiate bonding.
        NRF_LOG_INFO("[APP]Start encryption.\r\n");
        err_code = pm_conn_secure(m_conn_handle, false);
        if (err_code != NRF_ERROR_INVALID_STATE) {
            APP_ERROR_CHECK(err_code);
        }
    }
}

/**
 * @brief Timer event handler.
 *
 * @param[in]   p_context
 */
static void on_long_push_timer_timeout(void *p_context)
{
	NRF_LOG_INFO("[APP]Button long pushed.\r\n");

	m_long_pushed = true;
	on_button_evt(PIN_MAIN_SW_IN, APP_BUTTON_ACTION_LONG_PUSH);
}

/**
 * @brief Button event handler.
 *
 * @param[in]   pin_no
 * @param[in]   button_action
 */
static void on_button_evt(uint8_t pin_no, uint8_t button_action)
{
	ret_code_t err_code;

	switch (button_action) {
	case APP_BUTTON_ACTION_PUSH:
		{
			NRF_LOG_INFO("[APP]Button pushed(%d).\r\n", pin_no);
			if (pin_no == PIN_SCAN_SW_IN) {
				// do nothing...
			}
			else if (pin_no == PIN_MAIN_SW_IN) {
					err_code = app_timer_start(m_long_push_timer_id, LONG_PUSH_TIMEOUT, NULL);
					APP_ERROR_CHECK(err_code);
				}
			else {
				// do nothing...
			}

			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case APP_BUTTON_ACTION_RELEASE:
		{
			NRF_LOG_INFO("[APP]Button released(%d).\r\n", pin_no);

			if (m_long_pushed) {
				m_long_pushed = false;
				break;
			}
			if (pin_no == PIN_SCAN_SW_IN) {
				// Swipe待機
				// is available card?
				if (magstripedata_is_available_card()) {
					if (m_trigger_disabled_timer_sec == 0) {
						// head detected -> drive trigger.
						gpiote_drive_trigger();
						
						// trigger disabled timer.
						m_trigger_disabled_timer_sec = TRIGGER_DISABLED_TIME_OUT_SEC;
						err_code = app_timer_start(m_trigger_disabled_timer_id, TRIGGER_DISABLED_TIMER_INTERVAL, NULL);
						APP_ERROR_CHECK(err_code);
					}
				}
			}
			else if (pin_no == PIN_MAIN_SW_IN) {
				// timer stop.
				app_timer_stop(m_long_push_timer_id);

                // FIDO U2F固有の処理を実行
                NRF_LOG_INFO("[APP]PIN_MAIN_SW_IN short pushed(%d).\r\n", pin_no);
                if (ble_u2f_command_on_mainsw_event(&m_u2f) == true) {
                    break;
                }

				// switch the current card selection.
				magstripedata_switch_card_selection();
			}
			else {
				// do nothing...
			}

			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	case APP_BUTTON_ACTION_LONG_PUSH:
		{
			if (pin_no == PIN_SCAN_SW_IN) {
				// not supported...
			}
			else if (pin_no == PIN_MAIN_SW_IN) {
                // FIDO U2F固有の処理を実行
                NRF_LOG_INFO("[APP]PIN_MAIN_SW_IN long pushed(%d).\r\n", pin_no);
                if (ble_u2f_command_on_mainsw_long_push_event(&m_u2f) == true) {
                    break;
                }
			}
			else {
				// do nothing...
			}
			
			// No Operation Timer Reset.
			if (0 < m_no_operation_timer_sec) {
				m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
			}
		}
		break;
		
	default:
		// do nothing...
		break;
	}
}

/*******************************************************************************
 * private functions.
 ******************************************************************************/
/**
 * @brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
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

    // Create no operation timer.
    err_code = app_timer_create(&m_no_operation_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                on_no_operation_timer_timeout);
    APP_ERROR_CHECK(err_code);
    
    // Create trigger disabled timer.
    err_code = app_timer_create(&m_trigger_disabled_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                on_trigger_disabled_timer_timeout);
    APP_ERROR_CHECK(err_code);
    
    // Create card sync timer.
    err_code = app_timer_create(&m_card_sync_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                on_card_sync_timer_timeout);
    APP_ERROR_CHECK(err_code);
    
    // Create security request timer.
    err_code = app_timer_create(&m_sec_req_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                on_seq_req_timer_timeout);
    APP_ERROR_CHECK(err_code);
    
    // Create card sync timer.
    err_code = app_timer_create(&m_long_push_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                on_long_push_timer_timeout);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
    // GPIO pin Setting
    nrf_gpio_cfg_output(PIN_PWR_CONT_OUT);
    nrf_gpio_cfg_output(PIN_DRV_TRIG_OUT);

    //nrf_gpiote_task_configure(APP_GPIOTE_CHANNEL_0, PIN_PWR_CONT_OUT, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
    //nrf_gpiote_task_configure(APP_GPIOTE_CHANNEL_0, PIN_DRV_TRIG_OUT, NRF_GPIOTE_POLARITY_TOGGLE, NRF_GPIOTE_INITIAL_VALUE_LOW);
}

/**
 * @brief Power on continue.
 */
static void gpiote_power_cont(void)
{
    nrf_gpio_pin_set(PIN_PWR_CONT_OUT);
    nrf_delay_ms(2);
    nrf_gpio_pin_clear(PIN_PWR_CONT_OUT);
}

/**
 * @brief Drive trigger.
 */
static void gpiote_drive_trigger(void)
{
	// Drive Trigger.
	nrf_gpio_pin_set(PIN_DRV_TRIG_OUT);
	nrf_delay_ms(2);
	nrf_gpio_pin_clear(PIN_DRV_TRIG_OUT);
}

/**
 * @brief Function for initializing the button handler module.
 */
static void buttons_init(void)
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

/**@brief Function for starting timers.
 */
static void timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       uint32_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */
	
    ret_code_t err_code;

    // no operation timer.
	m_no_operation_timer_sec = NO_OPERATION_TIME_OUT_SEC;
    err_code = app_timer_start(m_no_operation_timer_id, NO_OPERATION_TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(on_pm_evt);
    APP_ERROR_CHECK(err_code);

    // FDS処理完了後のU2F処理を続行させる
    err_code = fds_register(ble_u2f_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);

    // LESC用のキーペアを生成する
    err_code = ble_u2f_pairing_lesc_generate_key_pair();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = softdevice_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Overwrite some of the default configurations for the BLE stack.
    ble_cfg_t ble_cfg;

    // Configure the maximum number of connections.
    memset(&ble_cfg, 0, sizeof(ble_cfg));
    ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
    ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
    ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
    err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // FIDO機能に対応できるようにするため、
    // デフォルトのBLE設定を変更する
    // Configure the maximum ATT MTU.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                 = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gatt_conn_cfg.att_mtu = NRF_BLE_GATT_MAX_MTU_SIZE;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GATT, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Configure the maximum event length.
    memset(&ble_cfg, 0x00, sizeof(ble_cfg));
    ble_cfg.conn_cfg.conn_cfg_tag                     = CONN_CFG_TAG;
    ble_cfg.conn_cfg.params.gap_conn_cfg.event_length = 320;
    ble_cfg.conn_cfg.params.gap_conn_cfg.conn_count   = BLE_GAP_CONN_COUNT_DEFAULT;
    err_code = sd_ble_cfg_set(BLE_CONN_CFG_GAP, &ble_cfg, ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = softdevice_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(on_ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(on_sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void ble_gap_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the GATT module.
 */
static void ble_gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, BLE_GATT_ATT_MTU_PERIPH_SIZE);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void ble_services_init(void)
{
	ret_code_t err_code;

	// Initialize Diverta One Card Service.
	ble_one_card_init_t	one_card_init;
	memset(&one_card_init, 0, sizeof(one_card_init));
	one_card_init.evt_handler = on_one_card_evt;

	err_code = ble_one_card_init(&one_card_init, &m_ble_one_card);
	APP_ERROR_CHECK(err_code);

    // Initialize FIDO U2F Service.
    err_code = ble_u2f_init_services(&m_u2f);
    APP_ERROR_CHECK(err_code);

	// Initialize Device Information Service.
    //  FIDO U2Fでは、以下の3項目を
    //  DISにより提供するのが必須.
    //   Manufacturer Name String
    //   Model Number String
    //   Firmware Revision String
	ble_dis_init_t dis_init;
	memset(&dis_init, 0, sizeof(dis_init));
	ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
	ble_srv_ascii_to_utf8(&dis_init.model_num_str    , MODEL_NUM);
	ble_srv_ascii_to_utf8(&dis_init.fw_rev_str       , FW_REV);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

	err_code = ble_dis_init(&dis_init);
	APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static void ble_conn_init(void)
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
    cp_init.error_handler                  = on_conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void ble_adv_init(void)
{
    ret_code_t             err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;

    // ペアリングモードでない場合は、
    // ディスカバリーができないよう設定
    advdata.flags = ble_u2f_pairing_advertising_flag();

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;
    
    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);

    // デフォルトから変更したBLE設定でアドバタイズを行うよう指示
    ble_advertising_conn_cfg_tag_set(CONN_CFG_TAG);
}

/*
 * @brief Function for starting advertising.
 */
static void ble_adv_start(void)
{
    NRF_LOG_INFO("ble_adv_start \r\n");
    ret_code_t err_code = ble_advertising_start(BLE_ADV_MODE_FAST);

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    ret_code_t err_code = sd_app_evt_wait();

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    // FIDO U2Fで使用しているLEDを消灯
    ble_u2f_led_light_LED(m_u2f.led_for_pairing_mode,  false);
    ble_u2f_led_light_LED(m_u2f.led_for_user_presence, false);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    NRF_LOG_DEBUG("[APP]calling sd_power_system_off() \r\n");
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for initializing the MagStripeData Library.
 */
static void magstripedata_init(void)
{
	card_manager_init();
	
    MagStripeData_init();
    MagStripeData_setSyncBitLength(MAGSTRIPEDATA_SYNC_BIT_LENGTH);
    MagStripeData_setTrackNumber(JIS_2);    // 仮
    MagStripeData_encode();                 // 仮
}

/**
 * @brief Function for switch the current card selection.
 */
static void magstripedata_switch_card_selection(void)
{
	bool	available_card	= false;
	uint8_t	card_num		= CARD_LIST_NUM;
	uint8_t	current_card_no = card_manager_selected_card_no();
	
	const card_info_t *p_selected_card = NULL;
	
	do {
		// current card info.
		p_selected_card = card_manager_selected_card_info(current_card_no);
		if (p_selected_card) {
			// available card?
			if (p_selected_card->card_type != ONECARD_UNUSED) {
				available_card = true;
				break;
			}
			// this card is emply.
			else {
				// find the next card.
				if (current_card_no + 1 < CARD_LIST_NUM) {
					current_card_no++;
				}
				else {
					// skip to #0.
					current_card_no = 1;
				}
			}
		}
		else {
			NRF_LOG_WARNING("[APP]Invalid card info.\r\n");
		}
	} while (card_num--);
	
	// no available card, is set #1.
	if (!available_card) {
		current_card_no = 1;
		
		p_selected_card = card_manager_selected_card_info(current_card_no);
	}
	
	// update selected card no.
	card_manager_set_selected_card_no(current_card_no);
	
	// Attribute value の更新.
	// # Selected Card Number.
	ble_one_card_set_selected_card_no(m_ble_one_card, current_card_no);
	NRF_LOG_INFO("[APP]current card no: %d\r\n", current_card_no);

	// transfer the MagstripeData.
	magstripedata_transfer(p_selected_card->card_type, 
						   p_selected_card->card_code.value, 
						   p_selected_card->card_code.len);
}

/**
 * @brief Function for transfer the MagStripeData.
 *
 * @param[in]	card_type			card type.
 * @param[in]	p_card_code			card code string.
 * @param[in]	card_code_len		card code length.
 */
static void magstripedata_transfer(uint8_t card_type, 
								   const uint8_t *p_card_code, 
								   uint16_t card_code_len)
{
	// Magstripe Data.
	MagStripeData_clear();
	MagStripeData_setTrackNumber((TrackNumber)card_type);
	for (int i = 0; i < card_code_len; i++) {
		if (p_card_code[i] != 0) {
			MagStripeData_add(p_card_code[i]);
		}
	}
	MagStripeData_encode();
	
	// F2F Data.
	unsigned char	*p_f2fdata			= MagStripeData_getF2FDataAddr();
	int				f2fdata_bit_len		= MagStripeData_getF2FDataBitLength();
	int				f2fdata_byte_len	= f2fdata_bit_len / 8;
	if ((f2fdata_bit_len % 8) != 0) {
		f2fdata_byte_len++;
	}
	NRF_LOG_INFO("[APP]f2f data bit  len : %d\r\n", f2fdata_bit_len);
	NRF_LOG_INFO("[APP]f2f data byte len : %d\r\n", f2fdata_byte_len);

	// transfer the F2F data via SPI.
	if (0 < f2fdata_byte_len) {
		//one_card_spi_master_init();
		one_card_spi_master_set_tx_data((uint8_t *)p_f2fdata, (uint16_t)f2fdata_byte_len);
		one_card_spi_master_transfer();
	}
}

/**
 * @brief Function for current card is available.
 */
static bool magstripedata_is_available_card(void)
{
	bool	is_available_card	= false;
	uint8_t current_card_no = card_manager_selected_card_no();
	NRF_LOG_INFO("[APP]current card no: %d\r\n", current_card_no);
	
	const card_info_t *p_selected_card = card_manager_selected_card_info(current_card_no);
	if (p_selected_card) {
		// available card?
		if (p_selected_card->card_type != ONECARD_UNUSED) {
			is_available_card = true;
		}
	}
	
	return is_available_card;
}

/**
 * @brief Function for generate the serial code.
 */
static void serial_code_generate(void)
{
	uint8_t code[12];
	memcpy(m_serial_code, SERIAL_CODE_BASE, SERIAL_CODE_LEN - 4);
	
	code[0] = m_serial_code[0] - '0';
	code[1] = m_serial_code[1] - '0';
	code[2] = m_serial_code[2] - '0';
	code[3] = m_serial_code[3] - '0';
	code[4] = m_serial_code[4] - '0';
	code[5] = m_serial_code[5] - '0';
	code[6] = m_serial_code[6] - '0';
	code[7] = m_serial_code[7] - '0';
	
	code[8] = (code[0] + code[2] + code[4] + code[6]) % 10;
	code[9] = (code[1] + code[3] + code[5] + code[7]) % 10;
	code[10] = (code[0] + code[1] + code[2] + code[3]) % 10;
	code[11] = (code[4] + code[5] + code[6] + code[7]) % 10;
	
	m_serial_code[8] = code[8] + '0';
	m_serial_code[9] = code[9] + '0';
	m_serial_code[10] = code[10] + '0';
	m_serial_code[11] = code[11] + '0';
	m_serial_code[12] = 0;
	
	NRF_LOG_INFO("[APP]Serial Code : %s.\r\n", (uint32_t)m_serial_code);
}

/**
 * @brief Function for compare the serial code.
 *
 * @param[in]	p_expected	expected serial code.
 * @param[in]	p_actual	actual serial code.
 */
static uint8_t serial_code_compare(const uint8_t *p_expected, const uint8_t *p_actual)
{
	uint8_t ret = 0;
	
	for (int i = 0; i < SERIAL_CODE_LEN; i++) {
		if (p_expected[i] != p_actual[i]) {
			ret++;
		}
	}
	
	return ret;
}

/**
 * @}
 */
