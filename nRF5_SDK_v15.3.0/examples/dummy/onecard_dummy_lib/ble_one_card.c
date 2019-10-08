#include "sdk_common.h"

#include "ble.h"
#include "ble_one_card.h"
#include "ble_srv_common.h"

#define NRF_LOG_MODULE_NAME ble_one_card
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_UUID_ONE_CARD_TX_CHARACTERISTIC 0x0003                      /**< The UUID of the TX Characteristic. */
#define BLE_UUID_ONE_CARD_RX_CHARACTERISTIC 0x0002                      /**< The UUID of the RX Characteristic. */

#define BLE_ONE_CARD_MAX_RX_CHAR_LEN        BLE_ONE_CARD_MAX_DATA_LEN   /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_ONE_CARD_MAX_TX_CHAR_LEN        BLE_ONE_CARD_MAX_DATA_LEN   /**< Maximum length of the TX Characteristic (in bytes). */

#define ONE_CARD_BASE_UUID                  {{0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x37, 0xa8, 0xe5, 0x11, 0x41, 0xe1, 0x00, 0x00, 0x2e, 0x42}}

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_one_card    One Card Service structure.
 * @param[in] p_ble_evt     Pointer to the event received from BLE stack.
 */
static void on_connect(ble_one_card_t * p_one_card, ble_evt_t const * p_ble_evt)
{
    ret_code_t                      err_code;
    ble_one_card_evt_t              evt;
    ble_gatts_value_t               gatts_val;
    uint8_t                         cccd_value[2];
    ble_one_card_client_context_t  *p_client = NULL;

    err_code = blcm_link_ctx_get(p_one_card->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gap_evt.conn_handle);
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_one_card->rx_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS) &&
        (p_one_card->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value)) {
        if (p_client != NULL) {
            p_client->is_notification_enabled = true;
        }

        memset(&evt, 0, sizeof(ble_one_card_evt_t));
        evt.type        = BLE_ONE_CARD_EVT_COMM_STARTED;
        evt.p_one_card  = p_one_card;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_one_card->data_handler(&evt);
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_one_card    One Card Service structure.
 * @param[in] p_ble_evt     Pointer to the event received from BLE stack.
 */
static void on_write(ble_one_card_t * p_one_card, ble_evt_t const * p_ble_evt)
{
    ret_code_t                      err_code;
    ble_one_card_evt_t              evt;
    ble_one_card_client_context_t  *p_client;
    ble_gatts_evt_write_t const    *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    err_code = blcm_link_ctx_get(p_one_card->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
    }

    memset(&evt, 0, sizeof(ble_one_card_evt_t));
    evt.p_one_card       = p_one_card;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if ((p_evt_write->handle == p_one_card->tx_handles.cccd_handle) &&
        (p_evt_write->len == 2)) {
        if (p_client != NULL) {
            if (ble_srv_is_notification_enabled(p_evt_write->data)) {
                p_client->is_notification_enabled = true;
                evt.type                          = BLE_ONE_CARD_EVT_COMM_STARTED;
            } else {
                p_client->is_notification_enabled = false;
                evt.type                          = BLE_ONE_CARD_EVT_COMM_STOPPED;
            }

            if (p_one_card->data_handler != NULL) {
                p_one_card->data_handler(&evt);
            }
        }

    } else if ((p_evt_write->handle == p_one_card->rx_handles.value_handle) &&
             (p_one_card->data_handler != NULL)) {
        evt.type                  = BLE_ONE_CARD_EVT_RX_DATA;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_one_card->data_handler(&evt);

    } else {
        // Do Nothing. This event is not relevant for this service.
    }
}


/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_one_card    One Card Service structure.
 * @param[in] p_ble_evt     Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_one_card_t * p_one_card, ble_evt_t const * p_ble_evt)
{
    ret_code_t                      err_code;
    ble_one_card_evt_t              evt;
    ble_one_card_client_context_t  *p_client;

    err_code = blcm_link_ctx_get(p_one_card->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched.",
                      p_ble_evt->evt.gatts_evt.conn_handle);
        return;
    }

    if (p_client->is_notification_enabled) {
        memset(&evt, 0, sizeof(ble_one_card_evt_t));
        evt.type        = BLE_ONE_CARD_EVT_TX_RDY;
        evt.p_one_card       = p_one_card;
        evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_one_card->data_handler(&evt);
    }
}


void ble_one_card_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL)) {
        return;
    }

    ble_one_card_t * p_one_card = (ble_one_card_t *)p_context;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_one_card, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_one_card, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_one_card, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_one_card_init(ble_one_card_t * p_one_card, ble_one_card_init_t const * p_one_card_init)
{
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         one_card_base_uuid = ONE_CARD_BASE_UUID;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_one_card);
    VERIFY_PARAM_NOT_NULL(p_one_card_init);

    // Initialize the service structure.
    p_one_card->data_handler = p_one_card_init->data_handler;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&one_card_base_uuid, &p_one_card->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_one_card->uuid_type;
    ble_uuid.uuid = BLE_UUID_ONE_CARD_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_one_card->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);

    // Add the RX Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_ONE_CARD_RX_CHARACTERISTIC;
    add_char_params.uuid_type                = p_one_card->uuid_type;
    add_char_params.max_len                  = BLE_ONE_CARD_MAX_RX_CHAR_LEN;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = true;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_one_card->service_handle, &add_char_params, &p_one_card->rx_handles);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // Add the TX Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_ONE_CARD_TX_CHARACTERISTIC;
    add_char_params.uuid_type         = p_one_card->uuid_type;
    add_char_params.max_len           = BLE_ONE_CARD_MAX_TX_CHAR_LEN;
    add_char_params.init_len          = sizeof(uint8_t);
    add_char_params.is_var_len        = true;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_one_card->service_handle, &add_char_params, &p_one_card->tx_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}
