/* 
 * File:   fido_hid_receive.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#ifndef FIDO_HID_RECEIVE_H
#define FIDO_HID_RECEIVE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool        fido_hid_receive_request_frame(uint8_t *p_buff, size_t size);
void        fido_hid_receive_on_request_received(void);
uint8_t     fido_hid_receive_header_CMD(void);
uint32_t    fido_hid_receive_header_CID(void);
uint8_t     fido_hid_receive_header_ERROR(void);
void       *fido_hid_receive_apdu(void);
uint8_t    *fido_hid_receive_apdu_data(void);
uint32_t    fido_hid_receive_apdu_Lc(void);


#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_RECEIVE_H */
