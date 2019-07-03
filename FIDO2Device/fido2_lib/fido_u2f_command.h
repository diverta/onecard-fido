#ifndef FIDO_U2F_COMMAND_H__
#define FIDO_U2F_COMMAND_H__

#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_u2f_command_hid_init(void);
void fido_u2f_command_send_response(uint8_t *response, size_t length);
void fido_u2f_command_msg(TRANSPORT_TYPE transport_type);
void fido_u2f_command_msg_send_response(void *const p_evt);
void fido_u2f_command_msg_report_sent(void);

bool fido_u2f_command_on_mainsw_event(void);
bool fido_u2f_command_on_mainsw_long_push_event(void);
//
// BLEサービス統合までの経過措置
//
uint8_t *fido_u2f_command_response_buffer(void);
size_t   fido_u2f_command_response_buffer_size(void);
size_t  *fido_u2f_command_response_length(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_U2F_COMMAND_H__

/** @} */
