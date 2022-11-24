/* 
 * File:   fido_crypto_hmac_sha1.c
 * Author: makmorit
 *
 * Created on 2022/11/24, 14:44
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_crypto_hmac_sha1
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// SHA1関連処理
//
#include "sha1.h"
#include "sha1_define.h"

//
// SHA1ハッシュ計算の元になる任意データを配列で保持
//
static uint8_t *challenge_addrs[2];
static size_t   challenge_sizes[2];

//
// HMAC-SHA1計算処理
//
void fido_crypto_hmac_sha1_calculate(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    challenge_addrs[0] = src_data;
    challenge_sizes[0] = src_data_size;
    if (src_data_2 != NULL && src_data_2_size > 0) {
        challenge_addrs[1] = src_data_2;
        challenge_sizes[1] = src_data_2_size;
        hmac_sha1_vector(key_data, key_data_size, 2, challenge_addrs, challenge_sizes, dest_data);

    } else {
        hmac_sha1_vector(key_data, key_data_size, 1, challenge_addrs, challenge_sizes, dest_data);
    }
}
