#ifndef BLE_U2F_COMMAND_H__
#define BLE_U2F_COMMAND_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "ble_u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

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

void ble_u2f_command_initialize_context(void);
void ble_u2f_command_finalize_context(void);
void ble_u2f_command_on_fs_evt(fds_evt_t const *const p_evt);
void ble_u2f_command_keepalive_timer_handler(void *p_context);
void ble_u2f_command_on_response_send_completed(void);
bool ble_u2f_command_on_mainsw_event(ble_u2f_t *p_u2f);
bool ble_u2f_command_on_mainsw_long_push_event(ble_u2f_t *p_u2f);

void fido_ble_command_set(enum COMMAND_TYPE c);
void fido_ble_command_on_request_received(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_COMMAND_H__

/** @} */
