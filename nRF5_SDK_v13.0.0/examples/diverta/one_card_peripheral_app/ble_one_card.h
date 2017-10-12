/**
 * @file ble_one_card.h
 *
 * One Card サービス.
 */
#ifndef BLE_ONE_CARD_H__
#define BLE_ONE_CARD_H__

/*******************************************************************************
 * include.
 ******************************************************************************/
#include "ble.h"


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
// 422E****-E141-11E5-A837-0800200C9A66
#define BLE_UUID_ONE_CARD_BASE         			{0x66, 0x9a, 0x0c, 0x20, 0x00, 0x08, 0x37, 0xa8, 0xe5, 0x11, 0x41, 0xe1, 0x00, 0x00, 0x2e, 0x42}


#define BLE_UUID_ONE_CARD_SERVICE_UUID_TYPE		BLE_UUID_TYPE_VENDOR_BEGIN
#define BLE_UUID_ONE_CARD_SERVICE				0x0000
#define BLE_UUID_ONE_CARD_CHAR_SELECT_CARD_NO	0x0001
#define BLE_UUID_ONE_CARD_CHAR_CARD_EVENT		0x0002
#define BLE_UUID_ONE_CARD_CHAR_SYNC_CARD_NO		0x0003
#define BLE_UUID_ONE_CARD_CHAR_CARD_CODE		0x0004
#define BLE_UUID_ONE_CARD_CHAR_CARD_TYPE 		0x0005
#define BLE_UUID_ONE_CARD_CHAR_CARD_TITLE 		0x0006
#define BLE_UUID_ONE_CARD_CHAR_CARD_SUB_TITLE	0x0007
#define BLE_UUID_ONE_CARD_CHAR_LOCK_TIME		0x0008
#define BLE_UUID_ONE_CARD_CHAR_SERIAL_CODE		0x0009
#define BLE_UUID_ONE_CARD_CHAR_TX				0x000A
#define BLE_UUID_ONE_CARD_CHAR_RX				0x000B

#define BLE_ONE_CARD_SERIAL_CODE_LEN			12			//!< 

/*******************************************************************************
 * type definition.
 ******************************************************************************/
/**
 * @brief One Card Server Event IDs.
 */
enum BLE_ONE_CARD_EVTS {
	BLE_ONE_CARD_EVT_SELECT_CARD_NO_READ,					//!< Read operation performed to Selected Card No Characteristic.
	BLE_ONE_CARD_EVT_CARD_EVENT_NOTIFY,						//!< T.B.D
	BLE_ONE_CARD_EVT_SYNC_CARD_NO_WRITE,					//!< Write operation performed to Sync Card No Characteristic.
	BLE_ONE_CARD_EVT_CARD_CODE_WRITE,						//!< Write operation performed to Card Code Characteristic.
	BLE_ONE_CARD_EVT_CARD_TYPE_WRITE,						//!< Write operation performed to Card Type Characteristic.
	BLE_ONE_CARD_EVT_CARD_TITLE_WRITE,						//!< Write operation performed to Card Title Characteristic.
	BLE_ONE_CARD_EVT_CARD_SUB_TITLE_WRITE,					//!< Write operation performed to Card Sub Title Characteristic.
	BLE_ONE_CARD_EVT_LOCK_TIME_READ,						//!< Read  operation performed to Lock Time.
	BLE_ONE_CARD_EVT_LOCK_TIME_WRITE,						//!< Write operation performed to Lock Time.
	BLE_ONE_CARD_EVT_SERIAL_CODE_READ,						//!< Read  operation performed to Serial Code.
	BLE_ONE_CARD_EVT_SERIAL_CODE_WRITE,						//!< Write operation performed to Serial Code.
	BLE_ONE_CARD_EVT_RX_WRITE,								//!< Write operation performed to Rx.

	BLE_ONE_CARD_EVT_MAX
};

/**
 * @brief イベントハンドラ.
 *
 * @param[in]   evt_id	イベントID.
 */
typedef void (*ble_one_card_evt_handler)(uint16_t evt_id);


/*******************************************************************************
 * structure.
 ******************************************************************************/
/**
 * @brief サービス初期化構造体.
 */
typedef struct ble_one_card_init_s {
    ble_one_card_evt_handler evt_handler;  					//!< イベントハンドラ.
} ble_one_card_init_t;

/**
 * @brief サービス構造体.
 */
typedef struct ble_one_card_s ble_one_card_t;


/*******************************************************************************
 * function prototype.
 ******************************************************************************/
/**
 * @brief サービス初期化.
 *
 * @param[in]	p_ble_one_card_init	サービス初期化構造体.
 * @param[out]	p_ble_one_card		サービス構造体.
 * @retval		NRF_SUCCESS 成功.
 */
uint32_t ble_one_card_init(const ble_one_card_init_t	*p_ble_one_card_init,
						   const ble_one_card_t			**p_ble_one_card);

/**
 * @brief BLEスタックイベントハンドラ.
 *
 * @param[in]	p_ble_evt			BLEイベント構造体.
 */
void ble_one_card_on_ble_evt(ble_evt_t *p_ble_evt);

/**
 * @brief UUID Type.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		UUID Type, see Types of UUID.
 */
uint8_t ble_one_card_uuid_type(const ble_one_card_t *p_ble_one_card);

/**
 * @brief Sync Card Number.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		Sync Card Number.
 */
uint8_t ble_one_card_sync_card_no(const ble_one_card_t *p_ble_one_card);

/**
 * @brief Card Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Card Code.
 * @param[out]	length				Card Code Length.
 */
void ble_one_card_card_code(const ble_one_card_t	*p_ble_one_card,
							const uint8_t			**p_value,
							uint16_t				*p_length);

/**
 * @brief Card Type.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		Card Type.
 */
uint8_t ble_one_card_card_type(const ble_one_card_t *p_ble_one_card);

/**
 * @brief Card Title.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Card Title.
 * @param[out]	length				Card Title Length.
 */
void ble_one_card_card_title(const ble_one_card_t	*p_ble_one_card,
							 const uint8_t			**p_value,
							 uint16_t				*p_length);

/**
 * @brief Card Sub Title.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Card Sub Title.
 * @param[out]	length				Card Sub Title Length.
 */
void ble_one_card_card_sub_title(const ble_one_card_t	*p_ble_one_card,
								 const uint8_t			**p_value,
								 uint16_t				*p_length);

/**
 * @brief Lock Time
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @retval		Lock Time.
 */
uint8_t ble_one_card_lock_time(const ble_one_card_t *p_ble_one_card);

/**
 * @brief Serial Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Serial Code.
 * @param[out]	length				Serial Code Length.
 */
void ble_one_card_serial_code(const ble_one_card_t	*p_ble_one_card,
							  const uint8_t 		**p_value,
							  uint16_t 				*p_length);

/**
 * @brief Rx.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[out]	p_value				Rx.
 * @param[out]	length				Rx Length.
 */
void ble_one_card_rx(const ble_one_card_t	*p_ble_one_card,
					 const uint8_t			**p_value,
					 uint16_t 				*p_length);

/**
 * @brief Selected Card Number.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 */
uint32_t ble_one_card_set_selected_card_no(const ble_one_card_t *p_ble_one_card, uint8_t card_no);

/**
 * @brief Serial Code.
 *
 * @param[in]	p_ble_one_card		サービス構造体.
 * @param[in]	p_value				Serial Code.
 * @param[in]	length				Serial Code Length.
 */
uint32_t ble_one_card_set_serial_code(const ble_one_card_t	*p_ble_one_card, 
									  const uint8_t			*p_value, 
									  uint16_t				length);

#endif // BLE_ONE_CARD_H__
