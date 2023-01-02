/* 
 * File:   fido_hid_channel.h
 * Author: makmorit
 *
 * Created on 2018/12/17, 13:23
 */
#ifndef FIDO_HID_CHANNEL_H
#define FIDO_HID_CHANNEL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void     fido_hid_channel_initialize_cid(void);
uint32_t fido_hid_channel_current_cid(void);
uint32_t fido_hid_channel_new_cid(void);
uint32_t fido_hid_channel_get_cid_from_bytes(uint8_t *cid);
void     fido_hid_channel_set_cid_bytes(uint8_t *cid, uint32_t _CID);
size_t   fido_hid_payload_length_get(void *msg);

void     fido_hid_channel_lock_start(uint32_t cid, uint8_t lock_param);
void     fido_hid_channel_lock_cancel(void);
uint32_t fido_hid_channel_lock_cid(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_CHANNEL_H */
