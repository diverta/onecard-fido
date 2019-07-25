#ifndef FIDO_U2F_COMMAND_H__
#define FIDO_U2F_COMMAND_H__

#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_u2f_command_hid_init(void);
void fido_u2f_command_send_response(uint8_t *response, size_t length);
void fido_u2f_command_msg(TRANSPORT_TYPE transport_type);
void fido_u2f_command_msg_response_sent(void);
void fido_u2f_command_keepalive_timer_handler(void);
void fido_u2f_command_tup_cancel(void);
bool fido_u2f_command_on_mainsw_event(void);
bool fido_u2f_command_on_mainsw_long_push_event(void);
void fido_u2f_command_ping(TRANSPORT_TYPE transport_type);
void fido_u2f_command_flash_failed(void);
void fido_u2f_command_flash_gc_done(void);
void fido_u2f_command_token_counter_record_updated(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_U2F_COMMAND_H__

/** @} */
