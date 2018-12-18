#ifndef HID_U2F_COMMAND_H__
#define HID_U2F_COMMAND_H__

#include <stdbool.h>
#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif

void u2f_hid_init_do_process(void);    
void u2f_version_do_process(void);
void u2f_register_do_process(void);
void u2f_register_send_response(fds_evt_t const *const p_evt);
void u2f_authenticate_do_process(void);
void u2f_authenticate_send_response(fds_evt_t const *const p_evt);

#ifdef __cplusplus
}
#endif

#endif // HID_U2F_COMMAND_H__

/** @} */
