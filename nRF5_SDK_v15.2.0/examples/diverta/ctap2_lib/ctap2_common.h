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
// CTAP2コマンドで共用する作業領域
// 
// RP IDのSHA-256ハッシュデータを保持
extern nrf_crypto_hash_sha256_digest_t ctap2_rpid_hash;
extern size_t                          ctap2_rpid_hash_size;

// flagsを保持
extern uint8_t ctap2_flags;

// signCountを保持
extern uint32_t ctap2_sign_count;

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_COMMON_H */

