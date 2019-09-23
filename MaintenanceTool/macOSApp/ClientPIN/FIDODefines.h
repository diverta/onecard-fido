//
//  FIDODefines.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/18.
//
#ifndef FIDODefines_h
#define FIDODefines_h

// CTAP2コマンドバイトの識別用
#define CTAP2_CMD_MAKE_CREDENTIAL       0x01
#define CTAP2_CMD_GET_ASSERTION         0x02
#define CTAP2_CMD_GETINFO               0x04
#define CTAP2_CMD_CLIENT_PIN            0x06
#define CTAP2_CMD_RESET                 0x07
#define CTAP2_CMD_GET_NEXT_ASSERTION    0x08

#define CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT   0x02
#define CTAP2_SUBCMD_CLIENT_PIN_SET             0x03
#define CTAP2_SUBCMD_CLIENT_PIN_CHANGE          0x04
#define CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN   0x05

// FIDO機能関連エラーステータス
#define CTAP1_ERR_SUCCESS               0x00
#define CTAP1_ERR_INVALID_COMMAND       0x01
#define CTAP1_ERR_INVALID_PARAMETER     0x02
#define CTAP1_ERR_INVALID_LENGTH        0x03
#define CTAP1_ERR_INVALID_SEQ           0x04
#define CTAP1_ERR_TIMEOUT               0x05
#define CTAP1_ERR_CHANNEL_BUSY          0x06
#define CTAP1_ERR_LOCK_REQUIRED         0x0a
#define CTAP1_ERR_INVALID_CHANNEL       0x0b
#define CTAP2_ERR_CBOR_PARSING          0x10
#define CTAP2_ERR_CBOR_UNEXPECTED_TYPE  0x11
#define CTAP2_ERR_INVALID_CBOR          0x12
#define CTAP2_ERR_INVALID_CBOR_TYPE     0x13
#define CTAP2_ERR_MISSING_PARAMETER     0x14
#define CTAP2_ERR_LIMIT_EXCEEDED        0x15
#define CTAP2_ERR_TOO_MANY_ELEMENTS     0x17
#define CTAP2_ERR_CREDENTIAL_EXCLUDED   0x19
#define CTAP2_ERR_PROCESSING            0x21
#define CTAP2_ERR_UNSUPPORTED_ALGORITHM 0x26
#define CTAP2_ERR_INVALID_OPTION        0x2c
#define CTAP2_ERR_KEEPALIVE_CANCEL      0x2d
#define CTAP2_ERR_NO_CREDENTIALS        0x2e
#define CTAP2_ERR_PIN_INVALID           0x31
#define CTAP2_ERR_PIN_BLOCKED           0x32
#define CTAP2_ERR_PIN_AUTH_INVALID      0x33
#define CTAP2_ERR_PIN_AUTH_BLOCKED      0x34
#define CTAP2_ERR_PIN_NOT_SET           0x35
#define CTAP2_ERR_PIN_POLICY_VIOLATION  0x37
#define CTAP1_ERR_OTHER                 0x7f
#define CTAP2_ERR_VENDOR_FIRST          0xf0
#define CTAP2_ERR_VENDOR_LAST           0xff
// 独自エラーステータス
#define CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST  (CTAP2_ERR_VENDOR_FIRST+0x0e)

// HIDコマンドバイト
#define HID_CMD_CTAPHID_PING        0x81
#define HID_CMD_MSG                 0x83
#define HID_CMD_CTAPHID_INIT        0x86
#define HID_CMD_CTAPHID_CBOR        0x90
#define HID_CMD_ERASE_SKEY_CERT     0xC0
#define HID_CMD_INSTALL_SKEY_CERT   0xC1
#define HID_CMD_GET_FLASH_STAT      0xC2
#define HID_CMD_GET_VERSION_INFO    0xC3
#define HID_CMD_UNKNOWN_ERROR       0xBF

// BLEコマンドバイト
#define BLE_CMD_MSG                 0x83

#endif /* FIDODefines_h */
