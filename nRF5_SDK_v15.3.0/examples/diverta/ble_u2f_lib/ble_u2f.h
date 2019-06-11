#ifndef BLE_U2F_H__
#define BLE_U2F_H__

#include "sdk_config.h"

#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// FIDO Authenticator固有の定義
//
#define NRF_BLE_GATT_MAX_MTU_SIZE   67

// FIDOアライアンス提供の共通ヘッダー
// "u2f.h"より抜粋
#include "fido_common.h"
#include "u2f.h"

// BLEパケット項目のサイズ
#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

// ここでBLEの送受信可能最大データ長を調整します
//   U2F Control Point、U2F Status のバッファ長も、
//   この長さに合わせます
#if defined(NRF_BLE_GATT_MAX_MTU_SIZE) && (NRF_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_U2F_MAX_DATA_LEN (NRF_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
    #define BLE_GATT_ATT_MTU_PERIPH_SIZE NRF_BLE_GATT_MAX_MTU_SIZE
#else
    #define BLE_U2F_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
    #define BLE_GATT_ATT_MTU_PERIPH_SIZE BLE_GATT_MTU_SIZE_DEFAULT
#endif

#define BLE_U2F_MAX_RECV_CHAR_LEN BLE_U2F_MAX_DATA_LEN
#define BLE_U2F_MAX_SEND_CHAR_LEN BLE_U2F_MAX_DATA_LEN


// BLE U2FサービスのUUID
#define BLE_UUID_U2F_SERVICE 0xFFFD

// 初期設定コマンド群(鍵・証明書の新規導入用等)
#define U2F_INS_INSTALL_INITBOND 0x41
#define U2F_INS_INSTALL_INITFSTR 0x42
#define U2F_INS_INSTALL_INITSKEY 0x43
#define U2F_INS_INSTALL_INITCERT 0x44
#define U2F_INS_INSTALL_PAIRING  0x45


typedef struct ble_u2f_s ble_u2f_t;

typedef void (*ble_u2f_data_handler_t) (ble_u2f_t * p_u2f, uint8_t * p_data, uint16_t length);

struct ble_u2f_s
{
    uint8_t                  uuid_type;
    uint16_t                 service_handle;

    ble_gatts_char_handles_t u2f_status_handles;
    ble_gatts_char_handles_t u2f_control_point_handles;
    ble_gatts_char_handles_t u2f_control_point_length_handles;
    ble_gatts_char_handles_t u2f_service_revision_bitfield_handles;
    ble_gatts_char_handles_t u2f_service_revision_handles;

    uint16_t                 conn_handle;
    ble_u2f_data_handler_t   data_handler;
};

// リクエストデータに含まれるBLEヘッダーを保持
typedef struct {
    uint8_t  CMD;
    uint32_t LEN;
    uint8_t  SEQ;

    // リクエストデータの検査中に
    // 確認されたエラーを保持
    uint8_t ERROR;

    // リクエストデータの検査中に
    // 設定されたステータスワードを保持
    uint16_t STATUS_WORD;

    // 後続リクエストがあるかどうかを保持
    bool CONT;
} BLE_HEADER_T;


// コマンドバッファに書き込まれた
// 文字列を識別するための定義
enum COMMAND_TYPE
{
    COMMAND_NONE = 0,
    COMMAND_INITBOND,
    COMMAND_CTAP2_COMMAND,
    COMMAND_U2F_REGISTER,
    COMMAND_U2F_AUTHENTICATE,
    COMMAND_U2F_VERSION,
    COMMAND_U2F_PING,
    COMMAND_CHANGE_PAIRING_MODE,
    COMMAND_PAIRING
};

// fstorageによりFlash ROM更新が完了時、
// 後続処理が使用するデータを共有
typedef struct
{
    enum COMMAND_TYPE command;
    ble_u2f_t        *p_u2f;
    BLE_HEADER_T     *p_ble_header;
    FIDO_APDU_T       *p_apdu;
    uint8_t          *apdu_data_buffer;
    uint16_t          apdu_data_buffer_length;
    uint8_t          *response_message_buffer;
    uint16_t          response_message_buffer_length;
    uint8_t          *signature_data_buffer;
    uint16_t          signature_data_buffer_length;
    uint8_t           keepalive_status_byte;
    uint8_t           user_presence_byte;
    uint32_t          token_counter;
} ble_u2f_context_t;


uint32_t ble_u2f_init(ble_u2f_t * p_u2f);
bool     ble_u2f_on_ble_evt(ble_u2f_t * p_u2f, ble_evt_t * p_ble_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_H__

/** @} */
