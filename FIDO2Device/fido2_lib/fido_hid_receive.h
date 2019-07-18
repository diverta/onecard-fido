/* 
 * File:   fido_hid_receive.h
 * Author: makmorit
 *
 * Created on 2018/11/21, 14:21
 */
#ifndef FIDO_HID_RECEIVE_H
#define FIDO_HID_RECEIVE_H

#include <stdbool.h>
#include "fido_common.h"

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

HID_HEADER_T *fido_hid_receive_header(void);
FIDO_APDU_T  *fido_hid_receive_apdu(void);

bool fido_hid_receive_request_frame(uint8_t *p_buff, size_t size, uint8_t *request_frame_buffer, size_t *request_frame_number);
void fido_hid_receive_on_request_received(uint8_t *request_frame_buffer, size_t request_frame_number);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_HID_RECEIVE_H */
