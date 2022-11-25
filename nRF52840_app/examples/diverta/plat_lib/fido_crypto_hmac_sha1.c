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

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG   false

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
static void hmac_sha1_calculate(uint8_t *key, size_t key_len, size_t num_elem, uint8_t *addr[], size_t *len, uint8_t *mac)
{
    // padding - key XORd with ipad/opad
    uint8_t k_pad[64];
    uint8_t tk[20];
    uint8_t *_adr[6];
    size_t  _len[6];
    size_t  i;

    // Fixed limit on the number of fragments to avoid having to
    // allocate memory (which could fail).
    if (num_elem > 5) {
        return;
    }

    // if key is longer than 64 bytes reset it to key = SHA1(key)
    if (key_len > 64) {
        sha1_hash_calculate(1, &key, &key_len, tk);
        key = tk;
        key_len = 20;
    }

    //
    // the HMAC_SHA1 transform looks like:
    //
    // SHA1(K XOR opad, SHA1(K XOR ipad, text))
    //
    // where K is an n byte key
    // ipad is the byte 0x36 repeated 64 times
    // opad is the byte 0x5c repeated 64 times
    // and text is the data being protected
    //
    // start out by storing key in ipad
    memset(k_pad, 0, sizeof(k_pad));
    memcpy(k_pad, key, key_len);

    // XOR key with ipad values
    for (i = 0; i < 64; i++) {
        k_pad[i] ^= 0x36;
    }

    // perform inner SHA1
    _adr[0] = k_pad;
    _len[0] = 64;
    for (i = 0; i < num_elem; i++) {
        _adr[i + 1] = addr[i];
        _len[i + 1] = len[i];
    }
    sha1_hash_calculate(1 + num_elem, _adr, _len, mac);
    memset(k_pad, 0, sizeof(k_pad));
    memcpy(k_pad, key, key_len);

    // XOR key with opad values
    for (i = 0; i < 64; i++) {
        k_pad[i] ^= 0x5c;
    }

    // perform outer SHA1
    _adr[0] = k_pad;
    _len[0] = 64;
    _adr[1] = mac;
    _len[1] = SHA1_MAC_LEN;
    sha1_hash_calculate(2, _adr, _len, mac);
}

void fido_crypto_hmac_sha1_calculate(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
#if LOG_HEXDUMP_DEBUG
    NRF_LOG_DEBUG("Secret for HMAC SHA-1 (%d bytes)", key_data_size);
    NRF_LOG_HEXDUMP_DEBUG(key_data, key_data_size);
    NRF_LOG_DEBUG("Challenge for HMAC SHA-1 (%d bytes)", src_data_size);
    NRF_LOG_HEXDUMP_DEBUG(src_data, src_data_size);
#endif

    challenge_addrs[0] = src_data;
    challenge_sizes[0] = src_data_size;
    if (src_data_2 != NULL && src_data_2_size > 0) {
        challenge_addrs[1] = src_data_2;
        challenge_sizes[1] = src_data_2_size;
        hmac_sha1_calculate(key_data, key_data_size, 2, challenge_addrs, challenge_sizes, dest_data);

    } else {
        hmac_sha1_calculate(key_data, key_data_size, 1, challenge_addrs, challenge_sizes, dest_data);
    }

#if LOG_HEXDUMP_DEBUG
    NRF_LOG_DEBUG("Calculated HMAC value (%d bytes)", SHA1_MAC_LEN);
    NRF_LOG_HEXDUMP_DEBUG(dest_data, SHA1_MAC_LEN);
#endif
}
