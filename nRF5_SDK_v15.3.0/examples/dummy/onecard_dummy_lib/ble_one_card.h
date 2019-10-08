#ifndef BLE_ONE_CARD_H__
#define BLE_ONE_CARD_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BLE_ONE_CARD_BLE_OBSERVER_PRIO
#define BLE_ONE_CARD_BLE_OBSERVER_PRIO 2
#endif

/**@brief   Macro for defining a ble_one_card instance.
 *
 * @param     _name Name of the instance.
 * @param[in] _one_card_max_clients Maximum number of clients connected at a time.
 * @hideinitializer
 */
#define BLE_ONE_CARD_DEF(_name, _one_card_max_clients)              \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),    \
                             (_one_card_max_clients),               \
                             sizeof(ble_one_card_client_context_t));\
    static ble_one_card_t _name =                                        \
    {                                                               \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage)   \
    };                                                              \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                             \
                         BLE_ONE_CARD_BLE_OBSERVER_PRIO,            \
                         ble_one_card_on_ble_evt,                   \
                         &_name)

#define BLE_UUID_ONE_CARD_SERVICE 0x0000
    
#define OPCODE_LENGTH        1
#define HANDLE_LENGTH        2

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the One Card service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_ONE_CARD_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
    #define BLE_ONE_CARD_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

typedef enum {
    BLE_ONE_CARD_EVT_RX_DATA,       /**< Data received. */
    BLE_ONE_CARD_EVT_TX_RDY,        /**< Service is ready to accept new data to be transmitted. */
    BLE_ONE_CARD_EVT_COMM_STARTED,  /**< Notification has been enabled. */
    BLE_ONE_CARD_EVT_COMM_STOPPED,  /**< Notification has been disabled. */
} ble_one_card_evt_type_t;

/* Forward declaration of the ble_one_card_t type. */
typedef struct ble_one_card_s ble_one_card_t;

typedef struct {
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} ble_one_card_evt_rx_data_t;

typedef struct {
    bool is_notification_enabled;   /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
} ble_one_card_client_context_t;

typedef struct {
    ble_one_card_evt_type_t         type;               /**< Event type. */
    ble_one_card_t                 *p_one_card;         /**< A pointer to the instance. */
    uint16_t                        conn_handle;        /**< Connection handle. */
    ble_one_card_client_context_t  *p_link_ctx;         /**< A pointer to the link context. */
    union {
        ble_one_card_evt_rx_data_t  rx_data;            /**< @ref BLE_ONE_CARD_EVT_RX_DATA event data. */
    } params;
} ble_one_card_evt_t;

typedef void (* ble_one_card_data_handler_t) (ble_one_card_evt_t * p_evt);

typedef struct {
    ble_one_card_data_handler_t     data_handler;       /**< Event handler to be called for handling received data. */
} ble_one_card_init_t;

struct ble_one_card_s {
    uint8_t                         uuid_type;          /**< UUID type for One Card Service Base UUID. */
    uint16_t                        service_handle;     /**< Handle of One Card Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        tx_handles;         /**< Handles related to the TX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        rx_handles;         /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t * const p_link_ctx_storage; /**< Pointer to link context storage with handles of all current connections and its context. */
    ble_one_card_data_handler_t     data_handler;       /**< Event handler to be called for handling received data. */
};

uint32_t ble_one_card_init(ble_one_card_t * p_one_card, ble_one_card_init_t const * p_one_card_init);
void     ble_one_card_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

#ifdef __cplusplus
}
#endif

#endif // BLE_ONE_CARD_H__

/** @} */
