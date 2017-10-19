/**
 * @file ble_one_card.c
 *
 * One Card サービス.
 */
#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_ONE_CARD)

/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_log.h"

#include "app_error.h"

#include "ble_srv_common.h"

#include "ble_one_card.h"


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define CHAR_UD_SELECT_CARD_NO		"Selected Card Number"
#define CHAR_UD_CARD_EVENT			"Card Event"
#define CHAR_UD_SYNC_CARD_NO		"Sync Card Number"
#define CHAR_UD_CARD_CODE			"Card Code"
#define CHAR_UD_CARD_TYPE			"Card Type"
#define CHAR_UD_CARD_TITLE			"Card Title"
#define CHAR_UD_CARD_SUB_TITLE		"Card Sub Title"
#define CHAR_UD_LOCK_TIME			"Lock Time"
#define CHAR_UD_SERIAL_CODE			"Serial Code"
#define CHAR_UD_TX					"Tx"
#define CHAR_UD_RX					"Rx"

#define GATTS_USER_MEM_DATA_LEN		((NRF_BLE_GATT_MAX_MTU_SIZE + 1) - (2 + 2 + 2))	
																		//!< ATT_MTU(23bytes), handle(2bytes), offset(2bytes), length(2bytes)
#define GATTS_USER_MEM_BLOCK_MAX	((NRF_BLE_GATT_MAX_MTU_SIZE + 1) * (BLE_GATTS_VAR_ATTR_LEN_MAX / GATTS_USER_MEM_DATA_LEN + 1))
																		//!< GATTS User memory block size.

/*******************************************************************************
 * structure.
 ******************************************************************************/
/**
 * @brief GATTS User memory layout for Queued Writes.
 */
#pragma pack(push, 1)
typedef struct ble_one_card_gatts_user_mem_s {
	uint16_t	handle;
	uint16_t	offset;
	uint16_t	length;
	uint8_t		data[1];
} ble_one_card_gatts_user_mem_t;
#pragma pack(pop)

/**
 * @brief Selected Card Number Characteristic.
 */
typedef struct ble_one_card_select_card_no_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Selected Card Number characteristic handles.
	uint8_t							value;								//!< value  : Selected Card Number attribute value.
	uint16_t						length;								//!< length : Selected Card Number attribute value length.
} ble_one_card_select_card_no_char_t;

/**
 * @brief Card Event Characteristic.
 */
typedef struct ble_one_card_card_event_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Card Event characteristic handles.
	uint8_t							value;								//!< value  : Card Event attribute value.
	uint16_t						length;								//!< length : Card Event attribute value length.
} ble_one_card_card_event_char_t;

/**
 * @brief Sync Card Number Characteristic.
 */
typedef struct ble_one_card_sync_card_no_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Sync Card Number characteristic handles.
	uint8_t							value;								//!< value  : Sync Card Number attribute value.
	uint16_t						length;								//!< length : Sync Card Number attribute value length.
} ble_one_card_sync_card_no_char_t;

/**
 * @brief Card Code Characteristic.
 * @note valueサイズは要調整。Service全体のメモリサイズが大きいと、NRF_ERROR_NO_MEMとなる。
 */
typedef struct ble_one_card_card_code_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Card Code characteristic handles.
	uint8_t							value[BLE_GATTS_VAR_ATTR_LEN_MAX/8];//!< value  : Card Code attribute value.
	uint16_t						length;								//!< length : Card Code attribute value length.
} ble_one_card_card_code_char_t;

/**
 * @brief Card Type Characteristic.
 */
typedef struct ble_one_card_card_type_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Card Type characteristic handles.
	uint8_t							value;								//!< value  : Card Type attribute value.
	uint16_t						length;								//!< length : Card Type attribute value length.
} ble_one_card_card_type_char_t;

/**
 * @brief Card Title Characteristic.
 * @note valueサイズは要調整。Service全体のメモリサイズが大きいと、NRF_ERROR_NO_MEMとなる。
 */
typedef struct ble_one_card_card_title_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Card Title characteristic handles.
	uint8_t							value[BLE_GATTS_VAR_ATTR_LEN_MAX/8];//!< value  : Card Title attribute value.
	uint16_t						length;								//!< length : Card Title attribute value length.
} ble_one_card_card_title_char_t;

/**
 * @brief Card Sub Title Characteristic.
 * @note valueサイズは要調整。Service全体のメモリサイズが大きいと、NRF_ERROR_NO_MEMとなる。
 */
typedef struct ble_one_card_card_sub_title_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Card Sub Title characteristic handles.
	uint8_t							value[BLE_GATTS_VAR_ATTR_LEN_MAX/8];//!< value  : Card Sub Title attribute value.
	uint16_t						length;								//!< length : Card Sub Title attribute value length.
} ble_one_card_card_sub_title_char_t;

/**
 * @brief Lock Time Characteristic.
 */
typedef struct ble_one_card_lock_time_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Sync Card Number characteristic handles.
	uint8_t							value;								//!< value  : Sync Card Number attribute value.
	uint16_t						length;								//!< length : Sync Card Number attribute value length.
} ble_one_card_lock_time_char_t;

/**
 * @brief Serial Code Characteristic.
 */
typedef struct ble_one_card_serial_code_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Serial Code characteristic handles.
	uint8_t							value[BLE_ONE_CARD_SERIAL_CODE_LEN];//!< value  : Serial Code attribute value.
	uint16_t						length;								//!< length : Serial Code attribute value length.
} ble_one_card_serial_code_char_t;

/**
 * @brief Tx Code Characteristic.
 * @note valueサイズは暫定。一先ずMTUに収まるサイズに。
 */
typedef struct ble_one_card_tx_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Tx characteristic handles.
	uint8_t							value[NRF_BLE_GATT_MAX_MTU_SIZE-3]; //!< value  : Tx attribute value.
	uint16_t						length;								//!< length : Tx attribute value length.
} ble_one_card_tx_char_t;

/**
 * @brief Rx Code Characteristic.
 * @note valueサイズは暫定。一先ずMTUに収まるサイズに。
 */
typedef struct ble_one_card_rx_char_s
{
	ble_gatts_char_handles_t		handles;							//!< handles: Rx characteristic handles.
	uint8_t							value[NRF_BLE_GATT_MAX_MTU_SIZE-3]; //!< value  : Rx attribute value.
	uint16_t						length;								//!< length : Rx attribute value length.
} ble_one_card_rx_char_t;

/**
 * @brief サービス構造体.
 */
typedef struct ble_one_card_s {
	uint8_t								uuid_type;						//!< UUIDタイプ.
	uint16_t							conn_handle;					//!< 接続ハンドル.

	uint16_t							service_handle;					//!< One Card Service.
	ble_one_card_select_card_no_char_t	select_card_no_char;			//!< Selected Card Number Characteristic.
	ble_one_card_card_event_char_t		card_event_char;				//!< Card Event Characteristic.
	ble_one_card_sync_card_no_char_t	sync_card_no_char;				//!< Sync Card Number Characteristic.
	ble_one_card_card_code_char_t		card_code_char;					//!< Card Code Characteristic.
	ble_one_card_card_type_char_t		card_type_char;					//!< Card Type Characteristic.
	ble_one_card_card_title_char_t		card_title_char;				//!< Card Title Characteristic.
	ble_one_card_card_sub_title_char_t	card_sub_title_char;			//!< Card Sub Title Characteristic.
	ble_one_card_lock_time_char_t		lock_time_char;					//!< Lock Time Characteristic.
	ble_one_card_serial_code_char_t		serial_code_char;				//!< Serial Code Characteristic.
	ble_one_card_tx_char_t				tx_char;						//!< Tx Characteristic.
	ble_one_card_rx_char_t				rx_char;						//!< Rx Characteristic.

	ble_one_card_evt_handler			evt_handler;					//!< イベントハンドラ .
} ble_one_card_t;


/*******************************************************************************
 * static fields.
 ******************************************************************************/
static ble_one_card_t 	m_ble_one_card;
static uint8_t			m_ble_one_card_gatts_user_mem_block[GATTS_USER_MEM_BLOCK_MAX];


/*******************************************************************************
 * function prototype.
 ******************************************************************************/

/*
 * service & characteristics.
 */
static uint32_t ble_one_card_add_service(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_characteristics(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_select_card_no_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_card_event_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_sync_card_no_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_card_code_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_card_type_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_card_title_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_card_sub_title_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_lock_time_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_serial_code_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_tx_char(ble_one_card_t *p_ble_one_card);
static uint32_t ble_one_card_add_rx_char(ble_one_card_t *p_ble_one_card);

/*
 * event handler.
 */
static void ble_one_card_on_connect(ble_evt_t *p_ble_evt);
static void ble_one_card_on_disconnect(ble_evt_t *p_ble_evt);
static void ble_one_card_on_user_mem_req(ble_evt_t *p_ble_evt);
static void ble_one_card_on_user_mem_rel(ble_evt_t *p_ble_evt);
static void ble_one_card_on_write(ble_evt_t *p_ble_evt);
static void ble_one_card_on_rw_authorize_request(ble_evt_t *p_ble_evt);
static void ble_one_card_on_sync_card_no_write(uint8_t value);
static void ble_one_card_on_card_code_write(uint8_t *p_value, uint16_t length);
static void ble_one_card_on_card_type_write(uint8_t value);
static void ble_one_card_on_card_title_write(uint8_t *p_value, uint16_t length);
static void ble_one_card_on_card_sub_title_write(uint8_t *p_value, uint16_t length);
static void ble_one_card_on_lock_time_write(uint8_t value);
static void ble_one_card_on_serial_code_write(uint8_t *p_value, uint16_t length);
static void ble_one_card_on_rx_write(uint8_t *p_value, uint16_t length);

static uint16_t ble_one_card_parse_gatts_user_mem(uint8_t *p_value, uint16_t length);
static void ble_one_card_parse_gatts_user_mem_to_attr_value(ble_one_card_gatts_user_mem_t *p_gatts_user_mem);


/*******************************************************************************
 * public function.
 ******************************************************************************/

/**
 * @brief サービス初期化.
 *
 * @param[in]	p_ble_one_card_init	サービス初期化構造体.
 * @param[out]	p_ble_one_card		サービス構造体.
 * @retval		NRF_SUCCESS	成功.
 */
uint32_t ble_one_card_init(const ble_one_card_init_t	*p_ble_one_card_init,
						   const ble_one_card_t 		**p_ble_one_card)
{
    uint32_t   err_code;

    // サービス構造体の初期化.
    memset(&m_ble_one_card, 0, sizeof(m_ble_one_card));
    m_ble_one_card.evt_handler	= p_ble_one_card_init->evt_handler;
    m_ble_one_card.conn_handle 	= BLE_CONN_HANDLE_INVALID;

    // サービスの登録.
    // # One Card.
    err_code = ble_one_card_add_service(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Selected Card Number.
    // # Card Event.
    // # Sync Card Number.
    // # Card Code.
    // # Card Type.
    // # Card Title.
    // # Card Sub Title.
    // # Lock Time.
    // # Serial Code.
    // # Tx.
    // # Rx.
    err_code = ble_one_card_add_characteristics(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // サービス構造データを参照渡し.
    *p_ble_one_card = &m_ble_one_card;

    return NRF_SUCCESS;
}

/**
 * @brief BLEスタックイベントハンドラ.
 *
 * @param[in]   p_ble_evt			BLEイベント構造体.
 */
void ble_one_card_on_ble_evt(ble_evt_t *p_ble_evt)
{
    switch (p_ble_evt->header.evt_id) {
    /*
     * Common events.
     */
    case BLE_EVT_USER_MEM_REQUEST:
		NRF_LOG_INFO("[BLE]BLE_EVT_USER_MEM_REQUEST\r\n");
		ble_one_card_on_user_mem_req(p_ble_evt);
        break;

    case BLE_EVT_USER_MEM_RELEASE:
    	NRF_LOG_INFO("[BLE]BLE_EVT_USER_MEM_RELEASE\r\n");
		ble_one_card_on_user_mem_rel(p_ble_evt);
        break;

    /*
     * GAP events.
     */
    case BLE_GAP_EVT_CONNECTED:
    	NRF_LOG_INFO("[BLE]BLE_GAP_EVT_CONNECTED\r\n");
        ble_one_card_on_connect(p_ble_evt);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
    	NRF_LOG_INFO("[BLE]BLE_GAP_EVT_DISCONNECTED\r\n");
        ble_one_card_on_disconnect(p_ble_evt);
        break;

    /*
     * GATT events.
     */
    case BLE_GATTS_EVT_WRITE:
    	NRF_LOG_INFO("[BLE]BLE_GATTS_EVT_WRITE\r\n");
		// MITM対応により、こちらは不要.
        //ble_one_card_on_write(p_ble_evt);
        break;

    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
		NRF_LOG_INFO("[BLE]BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST\r\n");
		ble_one_card_on_rw_authorize_request(p_ble_evt);
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**
 * @brief get UUID Type.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		UUID Type, see Types of UUID.
 */
uint8_t ble_one_card_uuid_type(const ble_one_card_t *p_ble_one_card)
{
	uint8_t uuid_type = BLE_UUID_TYPE_UNKNOWN;

	if (p_ble_one_card != NULL) {
		uuid_type = p_ble_one_card->uuid_type;
	}

	return uuid_type;
}

/**
 * @brief Sync Card Number.
 *
 * @param[in]   p_ble_one_card		サービス構造体.
 * @retval		Sync Card Number.
 */
uint8_t ble_one_card_sync_card_no(const ble_one_card_t *p_ble_one_card)
{
	uint8_t sync_card_no = 0;

	if (p_ble_one_card != NULL) {
		sync_card_no = p_ble_one_card->sync_card_no_char.value;
	}

	return sync_card_no;
}

/**
 * @brief Card Code.
 *
 * @param[in]   p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Card Code.
 * @param[out]	length				Card Code Length.
 */
void ble_one_card_card_code(const ble_one_card_t	*p_ble_one_card,
							const uint8_t			**p_value,
							uint16_t				*p_length)
{
	if (p_ble_one_card != NULL) {
		*p_value	= p_ble_one_card->card_code_char.value;
		*p_length	= p_ble_one_card->card_code_char.length;
	}
}

/**
 * @brief Card Type.
 *
 * @param[in]   p_ble_one_card		サービス構造体.
 * @retval		Card Type.
 */
uint8_t ble_one_card_card_type(const ble_one_card_t *p_ble_one_card)
{
	uint8_t card_type = 0;

	if (p_ble_one_card != NULL) {
		card_type = p_ble_one_card->card_type_char.value;
	}

	return card_type;
}

/**
 * @brief Card Title.
 *
 * @param[in]   p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Card Title.
 * @param[out]	length				Card Title Length.
 */
void ble_one_card_card_title(const ble_one_card_t	*p_ble_one_card,
							 const uint8_t			**p_value,
							 uint16_t				*p_length)
{
	if (p_ble_one_card != NULL) {
		*p_value	= p_ble_one_card->card_title_char.value;
		*p_length	= p_ble_one_card->card_title_char.length;
	}
}

/**
 * @brief Card Sub Title.
 *
 * @param[in]   p_ble_one_car		サービス構造体.
 * @param[out]	p_value				Card Sub Title.
 * @param[out]	length				Card Sub Title Length.
 */
void ble_one_card_card_sub_title(const ble_one_card_t	*p_ble_one_card,
								 const uint8_t			**p_value,
								 uint16_t				*p_length)
{
	if (p_ble_one_card != NULL) {
		*p_value	= p_ble_one_card->card_sub_title_char.value;
		*p_length	= p_ble_one_card->card_sub_title_char.length;
	}
}

/**
 * @brief Lock Time
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		Lock Time.
 */
uint8_t ble_one_card_lock_time(const ble_one_card_t *p_ble_one_card)
{
	uint8_t lock_time = 0;

	if (p_ble_one_card != NULL) {
		lock_time = p_ble_one_card->lock_time_char.value;
	}

	return lock_time;
}

/**
 * @brief Serial Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Serial Code.
 * @param[out]	length				Serial Code Length.
 */
void ble_one_card_serial_code(const ble_one_card_t	*p_ble_one_card,
							  const uint8_t 		**p_value,
							  uint16_t 				*p_length)
{
	if (p_ble_one_card != NULL) {
		*p_value	= p_ble_one_card->serial_code_char.value;
		*p_length	= p_ble_one_card->serial_code_char.length;
	}
}

/**
 * @brief Rx.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Rx.
 * @param[out]	length				Rx Length.
 */
void ble_one_card_rx(const ble_one_card_t	*p_ble_one_card,
					 const uint8_t			**p_value,
					 uint16_t 				*p_length)
{
	if (p_ble_one_card != NULL) {
		*p_value	= p_ble_one_card->rx_char.value;
		*p_length	= p_ble_one_card->rx_char.length;
	}
}

/**
 * @brief Selected Card Number.
 *
 * @param[in]   p_ble_one_card		サービス構造体.
 */
uint32_t ble_one_card_set_selected_card_no(const ble_one_card_t *p_ble_one_card, uint8_t card_no)
{
	uint32_t			err_code;
	ble_gatts_value_t	gatts_value;

	if (p_ble_one_card == &m_ble_one_card) {
		if (m_ble_one_card.select_card_no_char.value != card_no) {
			memset(&gatts_value, 0, sizeof(gatts_value));
			gatts_value.len		= sizeof(uint8_t);
			gatts_value.offset	= 0;
			gatts_value.p_value	= &card_no;

			// Update database.
			err_code = sd_ble_gatts_value_set(m_ble_one_card.conn_handle,
											  m_ble_one_card.select_card_no_char.handles.value_handle,
											  &gatts_value);
			if (err_code == NRF_SUCCESS) {
				// Save new value.
				m_ble_one_card.select_card_no_char.value = card_no;
			}
			return err_code;
		}
	}

	return NRF_SUCCESS;
}

/**
 * @brief Serial Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[in]	p_value				Serial Code.
 * @param[in]	length				Serial Code Length.
 */
uint32_t ble_one_card_set_serial_code(const ble_one_card_t	*p_ble_one_card, 
									  const uint8_t			*p_value, 
									  uint16_t				length)
{
	uint32_t			err_code;
	ble_gatts_value_t	gatts_value;

	if (p_ble_one_card == &m_ble_one_card) {
		memset(&gatts_value, 0, sizeof(gatts_value));
		gatts_value.len		= length;
		gatts_value.offset	= 0;
		gatts_value.p_value	= (uint8_t*)p_value;

		// Update database.
		err_code = sd_ble_gatts_value_set(m_ble_one_card.conn_handle,
										  m_ble_one_card.serial_code_char.handles.value_handle,
										  &gatts_value);
		if (err_code == NRF_SUCCESS) {
			// Save new value.
			for (int i = 0; i < length; i++) {
				m_ble_one_card.serial_code_char.value[i] = gatts_value.p_value[i];
			}
			m_ble_one_card.serial_code_char.length = length;
		}
		return err_code;
	}

	return NRF_SUCCESS;
}

/*******************************************************************************
 * ptivate function.
 ******************************************************************************/

/**
 * @brief サービス登録.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_service(ble_one_card_t *p_ble_one_card)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Base UUIDからUUID typeを取得.
    // # ベンダ固有サービスの登録には、128bitのBase UUIDが必要.
    // # Bluetooth SIGで定義されているサービスでは、
    //   16bitの短縮UUIDが使用が使用できるため、本処理は不要.
    ble_uuid128_t base_uuid = { BLE_UUID_ONE_CARD_BASE };
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_ble_one_card->uuid_type);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // サービスの登録.
    // # サービスへのハンドルを取得.
    ble_uuid.type = p_ble_one_card->uuid_type;
    ble_uuid.uuid = BLE_UUID_ONE_CARD_SERVICE;
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
    									&ble_uuid,
    									&p_ble_one_card->service_handle);

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**
 * @brief キャラクタリスティック登録.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_characteristics(ble_one_card_t *p_ble_one_card)
{
    uint32_t   err_code;

#if 0
    // キャラクタリスティックの登録.
    // # Selected Card Number.
    err_code = ble_one_card_add_select_card_no_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Card Event.
    err_code = ble_one_card_add_card_event_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Sync Card Number.
    err_code = ble_one_card_add_sync_card_no_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Card Code.
    err_code = ble_one_card_add_card_code_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Card Type.
    err_code = ble_one_card_add_card_type_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Card Title.
    err_code = ble_one_card_add_card_title_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Card Sub Title.
    err_code = ble_one_card_add_card_sub_title_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    // キャラクタリスティックの登録.
    // # Lock Time.
    err_code = ble_one_card_add_lock_time_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }
    
    // キャラクタリスティックの登録.
    // # Serial Code.
    err_code = ble_one_card_add_serial_code_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }
#endif
    
    // キャラクタリスティックの登録.
    // # Tx.
    err_code = ble_one_card_add_tx_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }
    
    // キャラクタリスティックの登録.
    // # Rx.
    err_code = ble_one_card_add_rx_char(&m_ble_one_card);
    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**
 * @brief キャラクタリスティック登録 : Selected Card Number.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_select_card_no_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    uint8_t				initial_card_no = 0;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_SELECT_CARD_NO;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Read.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read				= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_SELECT_CARD_NO;				// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_SELECT_CARD_NO);
    char_md.char_user_desc_max_size		= (uint8_t)strlen((char *)CHAR_UD_SELECT_CARD_NO);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Read permitted.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;

    // Attribute value.
    // # 1byte(hex).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.init_len	= sizeof(uint8_t);
    attr_char_value.init_offs	= 0;
    attr_char_value.max_len		= sizeof(uint8_t);
    attr_char_value.p_value		= &initial_card_no;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->select_card_no_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Card Event.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_card_event_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_CARD_EVENT;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Read, Notify.
    memset(&char_md, 0, sizeof(char_md));
    //char_md.char_props.read				= 1;
    char_md.char_props.notify			= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_CARD_EVENT;					// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_CARD_EVENT);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_CARD_EVENT);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Read, Write permitted.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;

    // Attribute value.
    // # 1byte(hex).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= sizeof(uint8_t);
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->card_event_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Sync Card Number.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_sync_card_no_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_SYNC_CARD_NO;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_SYNC_CARD_NO;				// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_SYNC_CARD_NO);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_SYNC_CARD_NO);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 1byte(hex).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= sizeof(uint8_t);
    attr_char_value.p_value		= &p_ble_one_card->sync_card_no_char.value;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->sync_card_no_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Card Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_card_code_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_CARD_CODE;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_CARD_CODE;					// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_CARD_CODE);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_CARD_CODE);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    // # Variable length.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.vlen	= 1;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 512bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= BLE_GATTS_VAR_ATTR_LEN_MAX/2;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->card_code_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Card Type.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_card_type_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_CARD_TYPE;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_CARD_TYPE;					// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_CARD_TYPE);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_CARD_TYPE);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 1byte(hex).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= sizeof(uint8_t);
    attr_char_value.p_value		= &p_ble_one_card->card_type_char.value;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->card_type_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Card Title.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_card_title_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_CARD_TITLE;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_CARD_TITLE;					// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_CARD_TITLE);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_CARD_TITLE);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    // # Variable length.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.vlen	= 1;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 512bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= BLE_GATTS_VAR_ATTR_LEN_MAX/8;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->card_title_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Card Sub Title.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_card_sub_title_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_CARD_SUB_TITLE;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_CARD_SUB_TITLE;				// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_CARD_SUB_TITLE);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_CARD_SUB_TITLE);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    // # Variable length.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.vlen	= 1;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 512bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= BLE_GATTS_VAR_ATTR_LEN_MAX/8;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->card_sub_title_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Lock Time.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_lock_time_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_LOCK_TIME;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read            	= 1;
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_LOCK_TIME;					// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_LOCK_TIME);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_LOCK_TIME);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth	= 1;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 1byte(hex).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= sizeof(uint8_t);
    attr_char_value.p_value		= &p_ble_one_card->lock_time_char.value;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->lock_time_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Serial Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_serial_code_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_SERIAL_CODE;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Read, Write.
    memset(&char_md, 0, sizeof(char_md));
    //char_md.char_props.read            	= 1;
    char_md.char_props.write        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_SERIAL_CODE;				// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_SERIAL_CODE);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_SERIAL_CODE);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Read  permitted(require encryption).
    // # Write permitted(require encryption).
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.vlen	= 1;
    //attr_md.rd_auth	= 1;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 12bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= BLE_ONE_CARD_SERIAL_CODE_LEN;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->serial_code_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Tx.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_tx_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_TX;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // Client Characteristic Configuration Description.
    // # Read Write.
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    // GATT Characteristic metadata.
    // # Notify.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.notify        	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_TX;							// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_TX);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_TX);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= &cccd_md;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Read Write.
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;

    // Attribute value.
    // # 20bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= NRF_BLE_GATT_MAX_MTU_SIZE - 3;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->tx_char.handles);
}

/**
 * @brief キャラクタリスティック登録 : Rx.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
static uint32_t ble_one_card_add_rx_char(ble_one_card_t *p_ble_one_card)
{
    ble_uuid_t          char_uuid;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t char_ud_md;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_t    attr_char_value;

    // UUID
    // # UUID typeは、サービス登録時に取得したものを使用.
    char_uuid.type = p_ble_one_card->uuid_type;
    char_uuid.uuid = BLE_UUID_ONE_CARD_CHAR_RX;

    // Characteristic User Descriptor.
    // # Read only.
    memset(&char_ud_md, 0, sizeof(char_ud_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&char_ud_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&char_ud_md.write_perm);
    char_ud_md.vloc = BLE_GATTS_VLOC_STACK;
    char_ud_md.vlen	= 1;

    // GATT Characteristic metadata.
    // # Write, Write without response.
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write        	= 1;
    char_md.char_props.write_wo_resp	= 1;
    char_md.p_char_user_desc        	= (uint8_t*)CHAR_UD_RX;							// user descriptor string.
    char_md.char_user_desc_size     	= (uint8_t)strlen((char *)CHAR_UD_RX);
    char_md.char_user_desc_max_size 	= (uint8_t)strlen((char *)CHAR_UD_RX);
    char_md.p_char_pf               	= NULL;
    char_md.p_user_desc_md          	= &char_ud_md;									// user descriptor.
    char_md.p_cccd_md               	= NULL;
    char_md.p_sccd_md               	= NULL;

    // Attribute metadata.
    // # Write permitted(require encryption).
    memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);
    attr_md.vloc	= BLE_GATTS_VLOC_STACK;
    attr_md.wr_auth	= 1;

    // Attribute value.
    // # 20bytes(utf8_s).
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid		= &char_uuid;					// UUID.
    attr_char_value.p_attr_md	= &attr_md;						// Attribute metadata.
    attr_char_value.max_len		= NRF_BLE_GATT_MAX_MTU_SIZE - 3;
    attr_char_value.p_value		= NULL;

    // キャラクタリスティックの登録.
    // # キャラクタリスティックへのハンドルを取得.
    return sd_ble_gatts_characteristic_add(p_ble_one_card->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ble_one_card->rx_char.handles);
}

/**
 * @brief Connect event.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_connect(ble_evt_t *p_ble_evt)
{
    m_ble_one_card.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**
 * @brief Disconnect event.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_disconnect(ble_evt_t *p_ble_evt)
{
    m_ble_one_card.conn_handle = BLE_CONN_HANDLE_INVALID;
}

/**
 * @brief User Memory Request event.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_user_mem_req(ble_evt_t *p_ble_evt)
{
	// event parameter.
	ble_evt_user_mem_request_t *p_evt_user_mem_req = &p_ble_evt->evt.common_evt.params.user_mem_request;

	if (p_evt_user_mem_req->type == BLE_USER_MEM_TYPE_GATTS_QUEUED_WRITES) {
		uint32_t				err_code;
		ble_user_mem_block_t	user_mem_block;
		user_mem_block.p_mem	= m_ble_one_card_gatts_user_mem_block;
		user_mem_block.len		= sizeof(m_ble_one_card_gatts_user_mem_block);
		err_code = sd_ble_user_mem_reply(m_ble_one_card.conn_handle, &user_mem_block);
		APP_ERROR_CHECK(err_code);
	}
}

/**
 * @brief User Memory Release event.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_user_mem_rel(ble_evt_t *p_ble_evt)
{
	// event parameter.
	ble_evt_user_mem_release_t *p_evt_user_mem_rel = &p_ble_evt->evt.common_evt.params.user_mem_release;

	if (p_evt_user_mem_rel->type == BLE_USER_MEM_TYPE_GATTS_QUEUED_WRITES) {
		// clean up.
		memset(p_evt_user_mem_rel->mem_block.p_mem, 0, p_evt_user_mem_rel->mem_block.len);
	}
}

/**
 * @brief Write operation performed.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_write(ble_evt_t *p_ble_evt)
{
// MITM対応により、こちらは不要.
#if 0
	// event parameter.
	ble_gatts_evt_write_t *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

	// Write Request.
	if (p_evt_write->op == BLE_GATTS_OP_WRITE_REQ) {
		NRF_LOG_INFO("[BLE]BLE_GATTS_OP_WRITE_REQ\r\n");
		// キャラクタリスティックに対するイベントの確認 : value of Sync Card Number.
		if (p_evt_write->handle == m_ble_one_card.sync_card_no_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Sync Card Number.\r\n");
			ble_one_card_on_sync_card_no_write(p_evt_write->data[0]);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Code.
		if (p_evt_write->handle == m_ble_one_card.card_code_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Card Code.\r\n");
			ble_one_card_on_card_code_write(p_evt_write->data, p_evt_write->len);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Type.
		if (p_evt_write->handle == m_ble_one_card.card_type_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Card Type.\r\n");
			ble_one_card_on_card_type_write(p_evt_write->data[0]);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Title.
		if (p_evt_write->handle == m_ble_one_card.card_title_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Card Title.\r\n");
			ble_one_card_on_card_title_write(p_evt_write->data, p_evt_write->len);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Sub Title.
		if (p_evt_write->handle == m_ble_one_card.card_sub_title_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Card Sub Title.\r\n");
			ble_one_card_on_card_sub_title_write(p_evt_write->data, p_evt_write->len);
		}
	}
	// Execute Write Request.
	// #GATTS Queued Writes: Stack handled, no attributes require authorization
	else if (p_evt_write->op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) {
		NRF_LOG_INFO("[BLE]BLE_GATTS_OP_EXEC_WRITE_REQ_NOW\r\n");

		uint16_t value_handle;
		value_handle = ble_one_card_parse_gatts_user_mem(m_ble_one_card_gatts_user_mem_block,
																sizeof(m_ble_one_card_gatts_user_mem_block));
		// キャラクタリスティックに対するイベントの確認 : value of Card Code.
		if (value_handle == m_ble_one_card.card_code_char.handles.value_handle) {
			ble_one_card_on_card_code_write(NULL, 0);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Title.
		if (value_handle == m_ble_one_card.card_title_char.handles.value_handle) {
			ble_one_card_on_card_title_write(NULL, 0);
		}
		// キャラクタリスティックに対するイベントの確認 : value of Card Sub Title.
		if (value_handle == m_ble_one_card.card_sub_title_char.handles.value_handle) {
			ble_one_card_on_card_sub_title_write(NULL, 0);
		}
	}
	else {
		// do nothing...
	}
#endif
}

/**
 * @brief Read/Write Authorization request.
 *
 * @param[in]	p_ble_evt	BLEイベント構造体.
 */
static void ble_one_card_on_rw_authorize_request(ble_evt_t *p_ble_evt)
{
	uint32_t err_code;
	ble_gatts_rw_authorize_reply_params_t auth_reply;
	memset(&auth_reply, 0, sizeof(auth_reply));

	// event parameter.
	ble_gatts_evt_rw_authorize_request_t *p_evt_auth_req = &p_ble_evt->evt.gatts_evt.params.authorize_request;

	if (p_evt_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
		NRF_LOG_INFO("[BLE]BLE_GATTS_AUTHORIZE_TYPE_READ\r\n");
		// キャラクタリスティックに対するイベントの確認 : value of Lock Time.
		if (p_evt_auth_req->request.read.handle == m_ble_one_card.lock_time_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Lock Time.\r\n");
			auth_reply.params.read.p_data		= &m_ble_one_card.lock_time_char.value;
			auth_reply.params.read.len			= m_ble_one_card.lock_time_char.length;
		}
		// キャラクタリスティックに対するイベントの確認 : value of Serial Code.
		else if (p_evt_auth_req->request.read.handle == m_ble_one_card.serial_code_char.handles.value_handle) {
			NRF_LOG_INFO("[BLE]Serial Code.\r\n");
			auth_reply.params.read.p_data		= m_ble_one_card.serial_code_char.value;
			auth_reply.params.read.len			= m_ble_one_card.serial_code_char.length;
		}
		else {
			// do nothing...
		}
		
		// set reply parameter.
		auth_reply.type						= BLE_GATTS_AUTHORIZE_TYPE_READ;
		auth_reply.params.read.gatt_status	= BLE_GATT_STATUS_SUCCESS;
		auth_reply.params.read.update		= 0;
	}
	else if (p_evt_auth_req->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
		NRF_LOG_INFO("[BLE]BLE_GATTS_AUTHORIZE_TYPE_WRITE\r\n");
		// Write Request.
		if (p_evt_auth_req->request.write.op == BLE_GATTS_OP_WRITE_REQ) {
			NRF_LOG_INFO("[BLE]BLE_GATTS_OP_WRITE_REQ\r\n");
			// キャラクタリスティックに対するイベントの確認 : value of Sync Card Number.
			if (p_evt_auth_req->request.write.handle == m_ble_one_card.sync_card_no_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Sync Card Number.\r\n");
				ble_one_card_on_sync_card_no_write(p_evt_auth_req->request.write.data[0]);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Code.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.card_code_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Card Code.\r\n");
				ble_one_card_on_card_code_write(p_evt_auth_req->request.write.data, p_evt_auth_req->request.write.len);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Type.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.card_type_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Card Type.\r\n");
				ble_one_card_on_card_type_write(p_evt_auth_req->request.write.data[0]);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Title.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.card_title_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Card Title.\r\n");
				ble_one_card_on_card_title_write(p_evt_auth_req->request.write.data, p_evt_auth_req->request.write.len);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Sub Title.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.card_sub_title_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Card Sub Title.\r\n");
				ble_one_card_on_card_sub_title_write(p_evt_auth_req->request.write.data, p_evt_auth_req->request.write.len);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Lock Time.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.lock_time_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Lock Time.\r\n");
				ble_one_card_on_lock_time_write(p_evt_auth_req->request.write.data[0]);
			}		
			// キャラクタリスティックに対するイベントの確認 : value of Auth Key.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.serial_code_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Serial Code.\r\n");
				ble_one_card_on_serial_code_write(p_evt_auth_req->request.write.data, p_evt_auth_req->request.write.len);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Tx.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.tx_char.handles.cccd_handle) {
				NRF_LOG_INFO("[BLE]Tx.\r\n");
				// 現状用途不明のため、無効.
			}
			// キャラクタリスティックに対するイベントの確認 : value of Rx.
			else if (p_evt_auth_req->request.write.handle == m_ble_one_card.rx_char.handles.value_handle) {
				NRF_LOG_INFO("[BLE]Rx.\r\n");
				ble_one_card_on_rx_write(p_evt_auth_req->request.write.data, p_evt_auth_req->request.write.len);
			}
			else {
				// do nothing...
			}
			
			// set reply parameter.
			auth_reply.type						= BLE_GATTS_AUTHORIZE_TYPE_WRITE;
			auth_reply.params.write.gatt_status	= BLE_GATT_STATUS_SUCCESS;
			auth_reply.params.write.update		= 1;
		}
		// Prepare Write Request.
		else if (p_evt_auth_req->request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ) {
			NRF_LOG_INFO("[BLE]BLE_GATTS_OP_PREP_WRITE_REQ\r\n");
			// set reply parameter.
			auth_reply.type						= BLE_GATTS_AUTHORIZE_TYPE_WRITE;
			auth_reply.params.write.gatt_status	= BLE_GATT_STATUS_SUCCESS;
			auth_reply.params.write.update		= 0;
		}
		// Execute Write Request.
		// #GATTS Queued Writes: Stack handled, no attributes require authorization
		else if (p_evt_auth_req->request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) {
			NRF_LOG_INFO("[BLE]BLE_GATTS_OP_EXEC_WRITE_REQ_NOW\r\n");

			uint16_t value_handle;
			value_handle = ble_one_card_parse_gatts_user_mem(m_ble_one_card_gatts_user_mem_block,
																	sizeof(m_ble_one_card_gatts_user_mem_block));

			// キャラクタリスティックに対するイベントの確認 : value of Card Code.
			if (value_handle == m_ble_one_card.card_code_char.handles.value_handle) {
				ble_one_card_on_card_code_write(NULL, 0);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Title.
			else if (value_handle == m_ble_one_card.card_title_char.handles.value_handle) {
				ble_one_card_on_card_title_write(NULL, 0);
			}
			// キャラクタリスティックに対するイベントの確認 : value of Card Sub Title.
			else if (value_handle == m_ble_one_card.card_sub_title_char.handles.value_handle) {
				ble_one_card_on_card_sub_title_write(NULL, 0);
			}
			else {
				// do nothing...
			}
			
			// set reply parameter.
			auth_reply.type						= BLE_GATTS_AUTHORIZE_TYPE_WRITE;
			auth_reply.params.write.gatt_status	= BLE_GATT_STATUS_SUCCESS;
			auth_reply.params.write.update		= 1;
		}
		else {
			// do nothing...
		}
	}
	else {
		// do nothing...
	}
	
	// Authorize Reply.
	err_code = sd_ble_gatts_rw_authorize_reply(m_ble_one_card.conn_handle, &auth_reply);
	APP_ERROR_CHECK(err_code);
}

/**
 * @brief Write event : value of Sync Card Number.
 *
 * @param[in]	value	受信データ.
 */
static void ble_one_card_on_sync_card_no_write(uint8_t value)
{
	// Sync Card Numberを更新.
	m_ble_one_card.sync_card_no_char.value = value;

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_SYNC_CARD_NO_WRITE);
	}
}

/**
 * @brief Write event : value of Card Code.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	length	受信データ長.
 */
static void ble_one_card_on_card_code_write(uint8_t *p_value, uint16_t length)
{
	// Card Codeを更新.
	if (p_value != NULL) {
		memset(m_ble_one_card.card_code_char.value, 0, sizeof(m_ble_one_card.card_code_char.value));
		memcpy(m_ble_one_card.card_code_char.value, p_value, length);
		m_ble_one_card.card_code_char.length = length;
	}

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_CARD_CODE_WRITE);
	}
}

/**
 * @brief Write event : value of Card Type.
 *
 * @param[in]	value	受信データ.
 */
static void ble_one_card_on_card_type_write(uint8_t value)
{
	// Card Typeを更新.
	m_ble_one_card.card_type_char.value = value;

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_CARD_TYPE_WRITE);
	}
}

/**
 * @brief Write event : value of Card Title.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	length	受信データ長.
 */
static void ble_one_card_on_card_title_write(uint8_t *p_value, uint16_t length)
{
	// Card Titleを更新.
	if (p_value != NULL) {
		memset(m_ble_one_card.card_title_char.value, 0, sizeof(m_ble_one_card.card_title_char.value));
		memcpy(m_ble_one_card.card_title_char.value, p_value, length);
		m_ble_one_card.card_title_char.length = length;
	}

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_CARD_TITLE_WRITE);
	}
}

/**
 * @brief Write event : value of Card Sub Title.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	length	受信データ長.
 */
static void ble_one_card_on_card_sub_title_write(uint8_t *p_value, uint16_t length)
{
	// Card Sub Titleを更新.
	if (p_value != NULL) {
		memset(m_ble_one_card.card_sub_title_char.value, 0, sizeof(m_ble_one_card.card_sub_title_char.value));
		memcpy(m_ble_one_card.card_sub_title_char.value, p_value, length);
		m_ble_one_card.card_sub_title_char.length = length;
	}

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_CARD_SUB_TITLE_WRITE);
	}
}

/**
 * @brief Write event : value of Lock Time.
 *
 * @param[in]	value	受信データ.
 */
static void ble_one_card_on_lock_time_write(uint8_t value)
{
	// Lock Timeを更新.
	m_ble_one_card.lock_time_char.value = value;

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_LOCK_TIME_WRITE);
	}
}

/**
 * @brief Write event : value of Serial Code.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	length	受信データ長.
 */
static void ble_one_card_on_serial_code_write(uint8_t *p_value, uint16_t length)
{
	// Serial Codeを更新.
	if (p_value != NULL) {
		memset(m_ble_one_card.serial_code_char.value, 0, sizeof(m_ble_one_card.serial_code_char.value));
		memcpy(m_ble_one_card.serial_code_char.value, p_value, length);
		m_ble_one_card.serial_code_char.length = length;
	}

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_SERIAL_CODE_WRITE);
	}
}

/**
 * @brief Write event : value of Rx.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	length	受信データ長.
 */
static void ble_one_card_on_rx_write(uint8_t *p_value, uint16_t length)
{
	// Rxを更新.
	if (p_value != NULL) {
		memset(m_ble_one_card.rx_char.value, 0, sizeof(m_ble_one_card.rx_char.value));
		memcpy(m_ble_one_card.rx_char.value, p_value, length);
		m_ble_one_card.rx_char.length = length;
	}

	// イベントハンドラの呼び出し
	if (m_ble_one_card.evt_handler != NULL) {
		m_ble_one_card.evt_handler(BLE_ONE_CARD_EVT_RX_WRITE);
	}
}

/**
 * @brief Write event : parses the gatts user memory.
 *
 * @param[in]	p_value	受信データ.
 * @param[in]	lenght	受信データ長.
 * @retval		value handle.
 */
static uint16_t ble_one_card_parse_gatts_user_mem(uint8_t *p_value, uint16_t length)
{
	ble_one_card_gatts_user_mem_t	*p_gatts_user_mem;
	uint8_t							*p_user_mem_block 	= p_value;
	uint16_t						value_handle		= (uint16_t)*p_value;

	do {
		p_gatts_user_mem = (ble_one_card_gatts_user_mem_t*)p_user_mem_block;
		//NRF_LOG_PRINTF("[BLE]:handle:%d\r\n", p_gatts_user_mem->handle);
		//NRF_LOG_PRINTF("[BLE]:offset:%d\r\n", p_gatts_user_mem->offset);
		//NRF_LOG_PRINTF("[BLE]:length:%d\r\n", p_gatts_user_mem->length);
		//NRF_LOG_PRINTF("[BLE]:data  :%c\r\n", p_gatts_user_mem->data[0]);

		if (0 < p_gatts_user_mem->length) {
			// copy to attribute value.
			ble_one_card_parse_gatts_user_mem_to_attr_value(p_gatts_user_mem);
			p_user_mem_block += sizeof(ble_one_card_gatts_user_mem_t) + (p_gatts_user_mem->length - 1);
		}
	} while (0 < p_gatts_user_mem->length);

	return value_handle;
}

/**
 * @brief Write event : parses the gatts user memory.
 *
 * @param[in]	p_gatts_user_mem	gatts user memory.
 */
static void ble_one_card_parse_gatts_user_mem_to_attr_value(ble_one_card_gatts_user_mem_t *p_gatts_user_mem)
{
	// キャラクタリスティックに対するイベントの確認 : value of Card Code.
	if (p_gatts_user_mem->handle == m_ble_one_card.card_code_char.handles.value_handle) {
		// clean up.
		if (p_gatts_user_mem->offset == 0) {
			m_ble_one_card.card_code_char.length = 0;
		}
		// copy to attribute value.
		memcpy(&m_ble_one_card.card_code_char.value[p_gatts_user_mem->offset],
			   p_gatts_user_mem->data,
			   p_gatts_user_mem->length);
		m_ble_one_card.card_code_char.length += p_gatts_user_mem->length;
	}
	// キャラクタリスティックに対するイベントの確認 : value of Card Title.
	else if (p_gatts_user_mem->handle == m_ble_one_card.card_title_char.handles.value_handle) {
		// clean up.
		if (p_gatts_user_mem->offset == 0) {
			m_ble_one_card.card_title_char.length = 0;
		}
		// copy to attribute value.
		memcpy(&m_ble_one_card.card_title_char.value[p_gatts_user_mem->offset],
			   p_gatts_user_mem->data,
			   p_gatts_user_mem->length);
		m_ble_one_card.card_title_char.length += p_gatts_user_mem->length;
	}
	// キャラクタリスティックに対するイベントの確認 : value of Card Sub Title.
	else if (p_gatts_user_mem->handle == m_ble_one_card.card_sub_title_char.handles.value_handle) {
		// clean up.
		if (p_gatts_user_mem->offset == 0) {
			m_ble_one_card.card_sub_title_char.length = 0;
		}
		// copy to attribute value.
		memcpy(&m_ble_one_card.card_sub_title_char.value[p_gatts_user_mem->offset],
			   p_gatts_user_mem->data,
			   p_gatts_user_mem->length);
		m_ble_one_card.card_sub_title_char.length += p_gatts_user_mem->length;
	}
	else {
		// do nothing...
	}
}
#endif // NRF_MODULE_ENABLED(BLE_ONE_CARD)
