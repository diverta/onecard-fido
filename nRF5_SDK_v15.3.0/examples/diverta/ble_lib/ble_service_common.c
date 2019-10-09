/* 
 * File:   ble_service_common.c
 * Author: makmorit
 *
 * Created on 2019/10/02, 12:58
 */
#include "nordic_common.h"
#include "nrf.h"

#include "peer_manager.h"
#include "peer_manager_handler.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_service_common
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_ble_event.h"
#include "fido_ble_peripheral.h"
#include "fido_ble_peripheral_timer.h"
#include "ble_service_central.h"

// for nrf_drv_usbd_is_enabled
#include "nrf_drv_usbd.h"

//業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

void ble_service_common_evt_handler(ble_evt_t const *p_ble_evt, void *p_context)
{
    ret_code_t err_code;
    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("BLE: Connected.");
            ble_peripheral_gap_connected(p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("BLE: Disconnected, reason %d.",
                          p_ble_evt->evt.gap_evt.params.disconnected.reason);
            ble_peripheral_gap_disconnected(p_ble_evt);
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
    if (fido_ble_peripheral_mode()) {
        fido_ble_evt_handler(p_ble_evt, p_context);
    }
}

void ble_service_common_gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt)
{
    UNUSED_PARAMETER(p_gatt);

    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        NRF_LOG_INFO("BLE: GATT ATT MTU on connection 0x%x changed to %d.",
                     p_evt->conn_handle,
                     p_evt->params.att_mtu_effective);
    }
}

//
// 初期化関連処理
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
    // FIDO Authenticator固有の処理
    if (fido_ble_pm_evt_handler(p_evt)) {
        return;
    }
    
    pm_handler_on_pm_evt(p_evt);
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id)
    {
        case PM_EVT_PEERS_DELETE_SUCCEEDED:
            fido_ble_peripheral_advertising_start();
            break;

        default:
            break;
    }
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

void ble_service_common_init(void)
{
    peer_manager_init();

    fido_ble_peripheral_init();
    ble_service_central_init();
}

void ble_service_common_enable_peripheral(void)
{
    // BLEペリフェラル始動タイマーを開始し、
    // 最初の１秒間でUSB接続されなかった場合は
    // BLEペリフェラル・モードに遷移
    fido_ble_peripheral_timer_start();
}

void ble_service_common_start_peripheral(void *p_context)
{
    UNUSED_PARAMETER(p_context);

    // USB接続・HIDサービス始動を確認
    bool enable_usbd = nrf_drv_usbd_is_enabled();
    NRF_LOG_DEBUG("USB HID is %s", 
        enable_usbd ? "active, BLE peripheral is inactive" : "inactive: starting BLE peripheral");

    if (enable_usbd == false) {
        // USB接続・HIDサービスが始動していない場合は
        // アドバタイジングを開始させ、
        // BLEペリフェラル・モードに遷移
        fido_ble_peripheral_start();
        return;
    }

    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();
}

void ble_service_common_disable_peripheral(void)
{
    if (fido_ble_peripheral_mode()) {
        // BLEペリフェラル稼働中にUSB接続された場合は、
        // ソフトデバイスを再起動
        NVIC_SystemReset();
    }
}
