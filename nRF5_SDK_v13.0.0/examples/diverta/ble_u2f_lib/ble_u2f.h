#ifndef BLE_U2F_H__
#define BLE_U2F_H__

#include "sdk_config.h"
#include "ble_stack_handler_types.h"

#include "ble.h"
#include "ble_srv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// FIDOアライアンス提供の共通ヘッダー
// "u2f.h"より抜粋
#define U2F_APPID_SIZE          32
#define U2F_CHAL_SIZE           32

#define U2F_POINT_UNCOMPRESSED  0x04

#define U2F_REGISTER            0x01
#define U2F_AUTHENTICATE        0x02
#define U2F_VERSION             0x03

#define U2F_VENDOR_FIRST        0x40
#define U2F_VENDOR_LAST         0xbf

#define U2F_AUTH_ENFORCE        0x03
#define U2F_AUTH_CHECK_ONLY     0x07
#define U2F_AUTH_FLAG_TUP       0x01

#define U2F_SW_NO_ERROR                 0x9000
#define U2F_SW_WRONG_DATA               0x6A80
#define U2F_SW_CONDITIONS_NOT_SATISFIED 0x6985
#define U2F_SW_COMMAND_NOT_ALLOWED      0x6986
#define U2F_SW_INS_NOT_SUPPORTED        0x6D00
#define U2F_SW_WRONG_LENGTH             0x6700
#define U2F_SW_CLA_NOT_SUPPORTED        0x6E00

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
    #warning NRF_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

#define BLE_U2F_MAX_RECV_CHAR_LEN BLE_U2F_MAX_DATA_LEN
#define BLE_U2F_MAX_SEND_CHAR_LEN BLE_U2F_MAX_DATA_LEN


// BLE U2FサービスのUUID
#define BLE_UUID_U2F_SERVICE 0xFFFD

// U2Fコマンドの識別用
#define U2F_COMMAND_PING      0x81
#define U2F_COMMAND_KEEPALIVE 0x82
#define U2F_COMMAND_MSG       0x83
#define U2F_COMMAND_ERROR     0xbf

// U2Fエラーステータスの識別用
#define U2F_ERR_INVALID_CMD 0x01
#define U2F_ERR_INVALID_LEN 0x03
#define U2F_ERR_INVALID_SEQ 0x04
#define U2F_ERR_OTHER       0x7f

// 鍵・証明書の新規導入用に用意
#define U2F_INS_INSTALL U2F_VENDOR_FIRST
#define U2F_INS_INSTALL_INITBOND 0x01
#define U2F_INS_INSTALL_INITFSTR 0x02
#define U2F_INS_INSTALL_INITSKEY 0x03
#define U2F_INS_INSTALL_INITCERT 0x04


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
    bool                     is_notification_enabled;
    ble_u2f_data_handler_t   data_handler;

    // BLE U2Fで使用するLEDのピン番号を保持
    uint32_t                 led_for_pairing_mode;
    uint32_t                 led_for_user_presence;
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

// リクエストデータに含まれるAPDU項目を保持
typedef struct {
    uint8_t  CLA;
    uint8_t  INS;
    uint8_t  P1;
    uint8_t  P2;
    uint32_t Lc;
    uint8_t *data;
    uint32_t data_length;
    uint32_t Le;
} U2F_APDU_T;

// コマンドバッファに書き込まれた
// 文字列を識別するための定義
enum COMMAND_TYPE
{
    COMMAND_NONE = 0,
    COMMAND_INITBOND,
    COMMAND_INITFSTR,
    COMMAND_INITSKEY,
    COMMAND_INITCERT,
    COMMAND_U2F_REGISTER,
    COMMAND_U2F_AUTHENTICATE,
    COMMAND_U2F_VERSION,
    COMMAND_U2F_PING,
    COMMAND_CHANGE_PAIRING_MODE
};

// fstorageによりFlash ROM更新が完了時、
// 後続処理が使用するデータを共有
typedef struct
{
    enum COMMAND_TYPE command;
    ble_u2f_t        *p_u2f;
    BLE_HEADER_T     *p_ble_header;
    U2F_APDU_T       *p_apdu;
    uint32_t         *securekey_buffer;
    uint16_t          securekey_buffer_length;
    uint8_t          *apdu_data_buffer;
    uint16_t          apdu_data_buffer_length;
    uint8_t          *response_message_buffer;
    uint16_t          response_message_buffer_length;
    uint8_t          *signature_data_buffer;
    uint16_t          signature_data_buffer_length;
    uint8_t           keepalive_status_byte;
    uint8_t           user_presence_byte;
    uint32_t          token_counter;
    bool              need_fdc_gc;
} ble_u2f_context_t;


uint32_t ble_u2f_init(ble_u2f_t * p_u2f);
bool     ble_u2f_on_ble_evt(ble_u2f_t * p_u2f, ble_evt_t * p_ble_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_H__

/** @} */
