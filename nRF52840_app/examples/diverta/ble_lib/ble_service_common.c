/* 
 * File:   ble_service_common.c
 * Author: makmorit
 *
 * Created on 2019/10/02, 12:58
 */
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_ble_gatt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "ble.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "fds.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_ble_event.h"
#include "ble_service_peripheral.h"
#include "ble_service_central.h"

//業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for Device name
#include "fido_board.h"

// BLEペリフェラルモードかどうかを保持
static bool ble_peripheral_mode = false;

bool ble_service_peripheral_mode(void)
{
    return ble_peripheral_mode;
}

void ble_service_peripheral_mode_set(bool b)
{
    ble_peripheral_mode = b;
}

//
// 初期化関連処理（BLE関連）
// 
#define APP_BLE_CONN_CFG_TAG                1           /**< A tag identifying the SoftDevice BLE configuration. */
#define APP_BLE_OBSERVER_PRIO               3           /**< Application's BLE observer priority. You shouldn't need to modify this value. */
NRF_BLE_GATT_DEF(m_gatt);                               /**< GATT module instance. */

static void ble_service_common_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code;
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("BLE: Connected.");
            ble_service_peripheral_gap_connected(p_ble_evt);
            ble_service_central_gap_connected(p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE: Disconnected, reason %d.",
                          p_ble_evt->evt.gap_evt.params.disconnected.reason);
            ble_service_peripheral_gap_disconnected(p_ble_evt);
            ble_service_central_gap_disconnected(p_ble_evt);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys = {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
    
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            NRF_LOG_DEBUG("BLE_GAP_EVT_SEC_PARAMS_REQUEST");
            break;
        
        case BLE_GAP_EVT_AUTH_KEY_REQUEST:
            NRF_LOG_DEBUG("BLE_GAP_EVT_AUTH_KEY_REQUEST");
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            NRF_LOG_DEBUG("BLE_GAP_EVT_LESC_DHKEY_REQUEST");
            break;

         case BLE_GAP_EVT_AUTH_STATUS:
             NRF_LOG_INFO("BLE_GAP_EVT_AUTH_STATUS: status=0x%x bond=0x%x lv4: %d kdist_own:0x%x kdist_peer:0x%x",
                          p_ble_evt->evt.gap_evt.params.auth_status.auth_status,
                          p_ble_evt->evt.gap_evt.params.auth_status.bonded,
                          p_ble_evt->evt.gap_evt.params.auth_status.sm1_levels.lv4,
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_own),
                          *((uint8_t *)&p_ble_evt->evt.gap_evt.params.auth_status.kdist_peer));
            ble_service_central_gap_evt_auth_status(p_ble_evt);
            break;

        case BLE_GAP_EVT_ADV_REPORT:
            ble_service_central_gap_adv_report(p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }

    // ペリフェラル・モードで動作する
    // FIDO Authenticator固有の処理
    if (ble_service_peripheral_mode()) {
        fido_ble_evt_handler(p_ble_evt, p_context);
    }
}

static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_app_ram_start_get(&ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_service_common_evt_handler, NULL);
}

static void ble_service_common_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    UNUSED_PARAMETER(p_gatt);

    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        NRF_LOG_INFO("BLE: GATT ATT MTU on connection 0x%x changed to %d.",
                     p_evt->conn_handle,
                     p_evt->params.att_mtu_effective);
    }
}

static void gatt_init(void)
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, ble_service_common_gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_central_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}
//
// ペアリング情報の全削除処理
//
static void (*erase_bonding_data_response_func)(bool) = NULL;

bool ble_service_common_erase_bond_data(void (*_response_func)(bool))
{
    // ペアリング情報削除後に実行される関数の参照を退避
    erase_bonding_data_response_func = _response_func;

    // 全てのペアリング情報を削除
    ret_code_t err_code = pm_peers_delete();
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("pm_peers_delete returns 0x%02x ", err_code);
        return false;
    }
    return true;
}

static void perform_erase_bonding_data_response_func(bool success)
{
    if (erase_bonding_data_response_func == NULL) {
        return;
    }

    // ペアリング情報削除後に実行される処理
    (*erase_bonding_data_response_func)(success);
    erase_bonding_data_response_func = NULL;
}

//
// 初期化関連処理（Peer Manager）
// 
#define SEC_PARAM_BOND                      1                                       /**< Perform bonding. */
#define SEC_PARAM_MITM                      0                                       /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                      1                                       /**< LE Secure Connections enabled. */
#define SEC_PARAM_KEYPRESS                  0                                       /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES           BLE_GAP_IO_CAPS_NONE                    /**< No I/O capabilities. */
#define SEC_PARAM_OOB                       0                                       /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE              7                                       /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE              16                                      /**< Maximum encryption key size. */

static void pm_evt_handler(pm_evt_t const * p_evt)
{
    // ペアリング情報削除時のイベントを最優先で処理
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_SUCCEEDED) {
        NRF_LOG_DEBUG("pm_peers_delete has completed successfully");
        perform_erase_bonding_data_response_func(true);
        return;
    }
    if (p_evt->evt_id == PM_EVT_PEERS_DELETE_FAILED) {
        NRF_LOG_ERROR("pm_peers_delete has failed");
        perform_erase_bonding_data_response_func(false);
        return;
    }

    if (ble_service_peripheral_mode()) {
        // FIDO Authenticator固有の処理
        if (fido_ble_pm_evt_handler(p_evt)) {
            return;
        }
    } else {
        // BLEセントラル固有の処理
        if (ble_service_central_pm_evt(p_evt)) {
            return;
        }
    }

    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);
}

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

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}

//
// 初期化関連処理（GAP）
//
#define MIN_CONN_INTERVAL                   MSEC_TO_UNITS( 40, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.05 seconds). */
#define MAX_CONN_INTERVAL                   MSEC_TO_UNITS(150, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.1875 second). */
#define SLAVE_LATENCY                       0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                    MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}

void ble_service_common_init(void)
{
    ble_stack_init();
    gatt_init();
    peer_manager_init();
    gap_params_init();

    ble_service_peripheral_init();
    ble_service_central_init();
}

void ble_service_common_disable_peripheral(void)
{
    if (ble_service_peripheral_mode()) {
        // BLEペリフェラル稼働中にUSB接続された場合は、
        // ソフトデバイスを再起動
        NVIC_SystemReset();
    }
}
