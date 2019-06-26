#ifndef FIDO_U2F_COMMAND_H__
#define FIDO_U2F_COMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

void hid_u2f_command_version(void);
void hid_u2f_command_msg(void);
void hid_u2f_command_msg_send_response(void *const p_evt);
void hid_u2f_command_msg_report_sent(void);

bool hid_u2f_command_on_mainsw_event(void);
bool hid_u2f_command_on_mainsw_long_push_event(void);
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
