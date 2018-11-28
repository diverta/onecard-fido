#ifndef HID_U2F_COMMAND_H__
#define HID_U2F_COMMAND_H__

#include <stdint.h>
#include <stdbool.h>

#include "fds.h"
#include "peer_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

void hid_u2f_command_on_report_received(void);
void hid_u2f_command_on_fs_evt(fds_evt_t const *const p_evt);

bool hid_u2f_command_on_mainsw_event(void);
bool hid_u2f_command_on_mainsw_long_push_event(void);

#ifdef __cplusplus
}
#endif

#endif // HID_U2F_COMMAND_H__

/** @} */
