/* 
 * File:   fido_nfc_receive.h
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:03
 */
#ifndef FIDO_NFC_RECEIVE_H
#define FIDO_NFC_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fido_common.h"

//
// 関数群
//
FIDO_APDU_T *nfc_fido_receive_apdu(void);
void nfc_fido_receive_request_frame(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_NFC_RECEIVE_H */
