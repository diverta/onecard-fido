/* 
 * File:   u2f.h
 * Author: makmorit
 *
 * Created on 2018/11/28, 16:03
 */

#ifndef U2F_H
#define U2F_H

#ifdef __cplusplus
extern "C" {
#endif

// FIDOアライアンス提供の共通ヘッダー
// "u2f.h"より抜粋
#define U2F_APPID_SIZE          32
#define U2F_CHAL_SIZE           32

#define U2F_POINT_UNCOMPRESSED  0x04

#define U2F_REGISTER            0x01
#define U2F_AUTHENTICATE        0x02
#define U2F_VERSION             0x03

#define U2F_VENDOR_FIRST        0x40
#define U2F_VENDOR_LAST         0xbf

#define U2F_AUTH_ENFORCE        0x03
#define U2F_AUTH_CHECK_ONLY     0x07
#define U2F_AUTH_FLAG_TUP       0x01

// Command status responses
#define U2F_SW_NO_ERROR                 0x9000
#define U2F_SW_WRONG_DATA               0x6A80
#define U2F_SW_CONDITIONS_NOT_SATISFIED 0x6985
#define U2F_SW_COMMAND_NOT_ALLOWED      0x6986
#define U2F_SW_INS_NOT_SUPPORTED        0x6D00
#define U2F_SW_WRONG_LENGTH             0x6700
#define U2F_SW_CLA_NOT_SUPPORTED        0x6E00


// U2Fコマンドの識別用
#define U2F_COMMAND_PING      0x81
#define U2F_COMMAND_KEEPALIVE 0x82
#define U2F_COMMAND_MSG       0x83
#define U2F_COMMAND_HID_LOCK  0x84
#define U2F_COMMAND_HID_INIT  0x86
#define U2F_COMMAND_HID_WINK  0x88
#define U2F_COMMAND_ERROR     0xbf

// U2Fエラーステータスの識別用
#define U2F_ERR_INVALID_CMD 0x01
#define U2F_ERR_INVALID_LEN 0x03
#define U2F_ERR_INVALID_SEQ 0x04
#define U2F_ERR_OTHER       0x7f

#ifdef __cplusplus
}
#endif

#endif /* U2F_H */

