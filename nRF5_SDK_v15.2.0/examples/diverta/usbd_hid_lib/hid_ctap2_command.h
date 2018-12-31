/* 
 * File:   hid_ctap2_command.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 13:36
 */
#ifndef HID_CTAP2_COMMAND_H
#define HID_CTAP2_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

void hid_ctap2_command_init(void);
void hid_ctap2_command_cbor(void);

bool hid_ctap2_command_on_mainsw_event(void);
bool hid_ctap2_command_on_mainsw_long_push_event(void);

#ifdef __cplusplus
}
#endif

#endif /* HID_CTAP2_COMMAND_H */

