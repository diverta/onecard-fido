#ifndef FIDO_U2F_COMMAND_H__
#define FIDO_U2F_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

// for Flash ROM event
#include "fido_flash_event.h"

void hid_u2f_command_version(void);
void hid_u2f_command_msg(void);
void hid_u2f_command_msg_send_response(fido_flash_event_t *const p_evt);
void hid_u2f_command_msg_report_sent(void);

bool hid_u2f_command_on_mainsw_event(void);
bool hid_u2f_command_on_mainsw_long_push_event(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_U2F_COMMAND_H__

/** @} */
