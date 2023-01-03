/* 
 * File:   fido_transport_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 16:28
 */
#ifndef FIDO_TRANSPORT_DEFINE_H
#define FIDO_TRANSPORT_DEFINE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// リクエストデータに含まれるBLEヘッダーを保持
typedef struct {
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
} BLE_HEADER_T;

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

// リクエストデータに含まれるAPDU項目を保持
typedef struct {
    uint8_t  CLA;
    uint8_t  INS;
    uint8_t  P1;
    uint8_t  P2;
    uint32_t Lc;
    uint8_t *data;
    uint32_t data_length;
    uint32_t Le;
} FIDO_APDU_T;

// APDUに格納できるデータ長の上限
#ifndef APDU_DATA_MAX_LENGTH
#define APDU_DATA_MAX_LENGTH 1024
#endif

//
// INITコマンドのレスポンスデータ編集領域
//   固定長（17バイト）
//   U2FHID_INIT、CTAPHID_INITで利用
//
typedef struct {
    uint8_t nonce[8];
    uint8_t cid[4];
    uint8_t version_id;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_build;
    uint8_t cflags;
} HID_INIT_RES_T;

//
// トランスポート種別
//
typedef enum _TRANSPORT_TYPE {
    TRANSPORT_NONE = 0,
    TRANSPORT_BLE,
    TRANSPORT_HID,
} TRANSPORT_TYPE;

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TRANSPORT_DEFINE_H */
