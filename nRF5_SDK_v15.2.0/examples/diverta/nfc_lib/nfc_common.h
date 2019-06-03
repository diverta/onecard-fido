/* 
 * File:   nfc_common.h
 * Author: makmorit
 *
 * Created on 2019/05/29, 11:43
 */
#ifndef NFC_COMMON_H
#define NFC_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NFC_APDU_BUFF_SIZE 512

// APDUヘッダー（5バイト）    
typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t lc;
} __attribute__((packed)) APDU_HEADER;
#define APDU_HEADER_SIZE 5

// APDUコマンドコード
#define APDU_FIDO_U2F_REGISTER        0x01
#define APDU_FIDO_U2F_AUTHENTICATE    0x02
#define APDU_FIDO_U2F_VERSION         0x03
#define APDU_FIDO_NFCCTAP_MSG         0x10
#define APDU_INS_SELECT               0xA4
#define APDU_INS_READ_BINARY          0xB0

// APDUレスポンスコード
#define SW_SUCCESS                    0x9000
#define SW_GET_RESPONSE               0x6100
#define SW_WRONG_LENGTH               0x6700
#define SW_COND_USE_NOT_SATISFIED     0x6985
#define SW_FILE_NOT_FOUND             0x6a82
#define SW_INS_INVALID                0x6d00
#define SW_INTERNAL_EXCEPTION         0x6f00

// アプリケーション選択用
#define AID_NDEF_TYPE_4             "\xD2\x76\x00\x00\x85\x01\x01"
#define AID_NDEF_MIFARE_TYPE_4      "\xD2\x76\x00\x00\x85\x01\x00"
#define AID_CAPABILITY_CONTAINER    "\xE1\x03"
#define AID_NDEF_TAG                "\xE1\x04"
#define AID_FIDO                    "\xa0\x00\x00\x06\x47\x2f\x00\x01"

// アプリケーション種別
typedef enum {
    APP_NOTHING = 0,
    APP_NDEF_TYPE_4,
    APP_MIFARE_TYPE_4,
    APP_CAPABILITY_CONTAINER,
    APP_NDEF_TAG,
    APP_FIDO,
} NFC_APPLETS;

#ifdef __cplusplus
}
#endif

#endif /* NFC_COMMON_H */
