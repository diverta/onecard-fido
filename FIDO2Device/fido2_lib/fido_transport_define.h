/* 
 * File:   fido_transport_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 16:28
 */
#ifndef FIDO_TRANSPORT_DEFINE_H
#define FIDO_TRANSPORT_DEFINE_H

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

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TRANSPORT_DEFINE_H */
