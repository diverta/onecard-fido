/* 
 * File:   hid_u2f_receive.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */

#ifndef HID_U2F_RECEIVE_H
#define HID_U2F_RECEIVE_H

#include <stdbool.h>
#include "u2f.h"

#ifdef __cplusplus
extern "C" {
#endif

// リクエストデータに含まれるHIDヘッダーを保持
typedef struct {
    uint32_t CID;
    uint8_t  CMD;
    uint32_t LEN;
    uint8_t  SEQ;

    // リクエストデータの検査中に
    // 確認されたエラーを保持
    uint8_t ERROR;

    // リクエストデータの検査中に
    // 設定されたステータスワードを保持
    uint16_t STATUS_WORD;

    // 後続リクエストがあるかどうかを保持
    bool CONT;
} HID_HEADER_T;

HID_HEADER_T *hid_fido_receive_hid_header(void);
FIDO_APDU_T   *hid_fido_receive_apdu(void);

void hid_fido_receive_request_data(uint8_t *request_frame_buffer, size_t request_frame_number);

#ifdef __cplusplus
}
#endif

#endif /* HID_U2F_RECEIVE_H */

