/* 
 * File:   fido_common.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_COMMON_H
#define FIDO_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#define CTAP2_ERR_SPEC_LAST             0xdf
#define CTAP2_ERR_EXTENSION_FIRST       0xe0
#define CTAP2_ERR_EXTENSION_LAST        0xef
#define CTAP2_ERR_VENDOR_FIRST          0xf0
#define CTAP2_ERR_VENDOR_LAST           0xff

// 独自エラーステータス
#define CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST  (CTAP2_ERR_VENDOR_FIRST+0x0e)

// 管理コマンドの識別用
#define MNT_COMMAND_BASE                0xc0
#define MNT_COMMAND_ERASE_SKEY_CERT     0xc0
#define MNT_COMMAND_INSTALL_SKEY_CERT   0xc1
#define MNT_COMMAND_GET_FLASH_STAT      0xc2
#define MNT_COMMAND_GET_APP_VERSION     0xc3
#define MNT_COMMAND_PREFERENCE_PARAM    0xc4
#define MNT_COMMAND_BOOTLOADER_MODE     0xc5
#define MNT_COMMAND_ERASE_BONDING_DATA  0xc6
#define MNT_COMMAND_SYSTEM_RESET        0xc7
#define MNT_COMMAND_INSTALL_ATTESTATION 0xc8

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

// FIDO関連 バージョン文字列
#define U2F_V2_VERSION_STRING      "U2F_V2"
#define FIDO_2_0_VERSION_STRING    "FIDO_2_0"

// トランスポート種別
typedef enum _TRANSPORT_TYPE {
    TRANSPORT_NONE = 0,
    TRANSPORT_BLE,
    TRANSPORT_HID,
    TRANSPORT_NFC
} TRANSPORT_TYPE;

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

// 関数群
void     fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word);
void     fido_set_uint32_bytes(uint8_t *p_dest_buffer, uint32_t bytes);
void     fido_set_uint16_bytes(uint8_t *p_dest_buffer, uint16_t bytes);
uint32_t fido_get_uint32_from_bytes(uint8_t *p_src_buffer);
uint64_t fido_get_uint64_from_bytes(uint8_t *p_src_buffer);
size_t   fido_calculate_aes_block_size(size_t buffer_size);
uint8_t *fido_extract_pubkey_in_certificate(uint8_t *cert_data, size_t cert_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMON_H */
