/* 
 * File:   ctap2_common.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:32
 */
#include "sdk_common.h"

#include "ctap2_common.h"

//
// CTAP2コマンドで共用する作業領域
// 
// RP IDのSHA-256ハッシュデータを保持
nrf_crypto_hash_sha256_digest_t ctap2_rpid_hash;
size_t                          ctap2_rpid_hash_size;

// flagsを保持
uint8_t ctap2_flags;

// signCountを保持
uint32_t ctap2_sign_count = 0;
