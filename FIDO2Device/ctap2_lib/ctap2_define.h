/* 
 * File:   ctap2_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 11:28
 */
#ifndef CTAP2_DEFINE_H
#define CTAP2_DEFINE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 各種処理用の定数
#define CTAP2_PRIVKEY_SIZE          32
#define CTAP2_PUBKEY_SIZE           64
#define CLIENT_DATA_HASH_SIZE       32
#define RP_ID_MAX_SIZE              128
#define RP_NAME_MAX_SIZE            32
#define USER_ID_MAX_SIZE            64
#define USER_NAME_MAX_SIZE          65
#define PUBKEY_CRED_SOURCE_MAX_SIZE 256
#define PUBKEY_CRED_TYPENM_MAX_SIZE 12
#define CREDENTIAL_ID_MAX_SIZE      256
#define AUTHENTICATOR_DATA_MAX_SIZE 384
#define ALLOW_LIST_MAX_SIZE         10
#define CRED_RANDOM_SIZE            32

// 各種処理用の定数（PIN関連）
#define PIN_AUTH_SIZE               16

//
// CTAP2をサポートする場合
// trueを設定
//
#define CTAP2_SUPPORTED true

// CTAP2コマンドの識別用
#define CTAP2_COMMAND_PING      0x81
#define CTAP2_COMMAND_LOCK      0x84
#define CTAP2_COMMAND_INIT      0x86
#define CTAP2_COMMAND_WINK      0x88
#define CTAP2_COMMAND_CBOR      0x90
#define CTAP2_COMMAND_CANCEL    0x91
#define CTAP2_COMMAND_ERROR     0xbf
#define CTAP2_COMMAND_KEEPALIVE 0xbb

// CTAP2コマンドバイトの識別用
#define CTAP2_CMD_MAKE_CREDENTIAL       0x01
#define CTAP2_CMD_GET_ASSERTION         0x02
#define CTAP2_CMD_GETINFO               0x04
#define CTAP2_CMD_CLIENT_PIN            0x06
#define CTAP2_CMD_RESET                 0x07
#define CTAP2_CMD_GET_NEXT_ASSERTION    0x08

// CTAPHID_INITのオプション識別用
#define CTAP2_CAPABILITY_WINK   0x01
#define CTAP2_CAPABILITY_LOCK   0x02
#define CTAP2_CAPABILITY_CBOR   0x04
#define CTAP2_CAPABILITY_NMSG   0x08

// CTAP2で許容されるメッセージの最大サイズ
#define CTAP2_MAX_MESSAGE_SIZE  1200

// キープアライブステータス
#define CTAP2_STATUS_UPNEEDED   0x02

// キープアライブ・タイマー
#define CTAP2_KEEPALIVE_INTERVAL_MSEC       500

// ユーザー所在確認タイムアウト（３０秒）
#define USER_PRESENCE_VERIFY_TIMEOUT_MSEC   30000

//
// CTAP2コマンドで共用する構造体
// 
typedef struct {
    uint8_t id[RP_ID_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[RP_NAME_MAX_SIZE];
} CTAP_RP_ID_T;

typedef struct {
    uint8_t id[USER_ID_MAX_SIZE];
    uint8_t id_size;
    uint8_t name[USER_NAME_MAX_SIZE];
} CTAP_USER_ENTITY_T;

typedef struct {
    uint8_t publicKeyCredentialTypeName[16];
    uint8_t publicKeyCredentialType;
    int32_t COSEAlgorithmIdentifier;
} CTAP_PUBKEY_CRED_PARAM_T;

typedef struct {
    uint8_t rk;
    uint8_t uv;
    uint8_t up;
} CTAP_OPTIONS_T;

typedef struct {
    uint8_t type;
    size_t  credential_id_size;
    uint8_t credential_id[CREDENTIAL_ID_MAX_SIZE];
} CTAP_CREDENTIAL_DESC_T;

typedef struct {
    uint8_t                size;
    CTAP_CREDENTIAL_DESC_T list[ALLOW_LIST_MAX_SIZE];
} CTAP_ALLOW_LIST_T;

typedef struct {
    int kty;
    int alg;
    int crv;
    struct {
        uint8_t x[32];
        uint8_t y[32];
    } key;
} CTAP_COSE_KEY;

typedef struct {
    bool           hmac_secret_parsed;
    uint8_t        saltLen;
    uint8_t        saltEnc[64];
    uint8_t        saltAuth[16];
    CTAP_COSE_KEY  keyAgreement;
} CTAP_HMAC_SECRET_T;

typedef struct {
    bool               hmac_secret_requested;
    CTAP_HMAC_SECRET_T hmac_secret;
} CTAP_EXTENSIONS_T;

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_DEFINE_H */
