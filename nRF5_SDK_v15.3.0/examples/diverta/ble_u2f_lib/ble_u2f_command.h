#ifndef BLE_U2F_COMMAND_H__
#define BLE_U2F_COMMAND_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "ble_gatts.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

// 初期設定コマンド群(鍵・証明書の新規導入用等)
#define U2F_INS_INSTALL_INITBOND 0x41
#define U2F_INS_INSTALL_INITFSTR 0x42
#define U2F_INS_INSTALL_INITSKEY 0x43
#define U2F_INS_INSTALL_INITCERT 0x44
#define U2F_INS_INSTALL_PAIRING  0x45

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

ble_u2f_context_t *get_ble_u2f_context(void);
void ble_u2f_command_initialize_context(void);
void ble_u2f_command_finalize_context(void);
void ble_u2f_command_on_ble_evt_write(ble_u2f_t *p_u2f, ble_gatts_evt_write_t *p_evt_write);
void ble_u2f_command_on_fs_evt(fds_evt_t const *const p_evt);
void ble_u2f_command_keepalive_timer_handler(void *p_context);
void ble_u2f_command_on_response_send_completed(void);
bool ble_u2f_command_on_mainsw_event(ble_u2f_t *p_u2f);
bool ble_u2f_command_on_mainsw_long_push_event(ble_u2f_t *p_u2f);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_COMMAND_H__

/** @} */
