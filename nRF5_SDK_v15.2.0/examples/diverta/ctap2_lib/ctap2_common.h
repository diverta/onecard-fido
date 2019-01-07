/* 
 * File:   ctap2_common.h
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#ifndef CTAP2_COMMON_H
#define CTAP2_COMMON_H

#include "nrf_crypto_hash.h"

#ifdef __cplusplus
extern "C" {
#endif

// 各種処理用の定数
#define CLIENT_DATA_HASH_SIZE       32
#define RP_ID_MAX_SIZE              128
#define RP_NAME_MAX_SIZE            32
#define USER_ID_MAX_SIZE            64
#define USER_NAME_MAX_SIZE          65
#define PUBKEY_CRED_SOURCE_MAX_SIZE 128
#define PUBKEY_CRED_TYPENM_MAX_SIZE 12
#define CREDENTIAL_ID_MAX_SIZE      80
#define AUTHENTICATOR_DATA_MAX_SIZE 256
#define ALLOW_LIST_MAX_SIZE         10

//
// CTAP2をサポートする場合
// trueを設定
//
#define CTAP2_SUPPORTED true

// CTAP2コマンドの識別用
#define CTAP2_COMMAND_PING      0x81
#define CTAP2_COMMAND_INIT      0x86
#define CTAP2_COMMAND_CBOR      0x90
#define CTAP2_COMMAND_ERROR     0xbf

// CTAP2コマンドバイトの識別用
#define CTAP2_CMD_MAKE_CREDENTIAL       0x01
#define CTAP2_CMD_GET_ASSERTION         0x02
#define CTAP2_CMD_GETINFO               0x04
#define CTAP2_CMD_CLIENT_PIN            0x06
#define CTAP2_CMD_GET_NEXT_ASSERTION    0x08

// CTAPHID_INITのオプション識別用
#define CTAP2_CAPABILITY_CBOR   0x04
#define CTAP2_CAPABILITY_NMSG   0x08

// CTAP2で許容されるメッセージの最大サイズ
#define CTAP2_MAX_MESSAGE_SIZE  1200

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
    CTAP_USER_ENTITY_T user_entity;
} CTAP_CREDENTIAL_DESC_T;

//
// CTAP2コマンドで共用する作業領域
// 
// RP IDのSHA-256ハッシュデータを保持
extern nrf_crypto_hash_sha256_digest_t ctap2_rpid_hash;
extern size_t                          ctap2_rpid_hash_size;

// flagsを保持
extern uint8_t ctap2_flags;

// signCountを保持
extern uint32_t ctap2_sign_count;

// Public Key Credential Sourceを保持
extern uint8_t pubkey_cred_source[PUBKEY_CRED_SOURCE_MAX_SIZE];
extern size_t  pubkey_cred_source_block_size;

// credentialIdを保持
extern uint8_t credential_id[CREDENTIAL_ID_MAX_SIZE];
extern size_t  credential_id_size;

// credentialPublicKeyを保持
extern uint8_t credential_pubkey[CREDENTIAL_ID_MAX_SIZE];
extern size_t  credential_pubkey_size;

// Authenticator dataを保持
extern uint8_t authenticator_data[AUTHENTICATOR_DATA_MAX_SIZE];
extern size_t  authenticator_data_size;

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_COMMON_H */

