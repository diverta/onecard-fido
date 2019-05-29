/* 
 * File:   nfc_fido_receive.h
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:03
 */
#ifndef NFC_FIDO_RECEIVE_H
#define NFC_FIDO_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void nfc_fido_receive_request_frame(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* NFC_FIDO_RECEIVE_H */
