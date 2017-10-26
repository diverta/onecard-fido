#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_util.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_crypto_ecb"
#include "nrf_log.h"

// AES ECBで使用する作業用エリア
#define ECB_BLOCK_LENGTH 16
static nrf_ecb_hal_data_t m_ecb_data;
static uint8_t block_cipher[ECB_BLOCK_LENGTH];

// AES ECBで使用する初期化ベクターとパスコード
// FIXME: 
//   この値は本来外部から与えるものではないため、
//   トークンの初回使用時にランダマイズされたものを生成し、
//   以降はFlash ROMに保管したものを使用できるように
//   後日改修予定
//
uint8_t m_initialization_vector[ECB_BLOCK_LENGTH] = {
    0x38, 0xa7, 0xc3, 0x8e, 0x6b, 0xb8, 0x3f, 0x79, 
    0xad, 0x7b, 0x7b, 0xba, 0x0c, 0x58, 0x4c, 0x58
};
uint8_t m_ecb_key[ECB_BLOCK_LENGTH] = {
    0xfe, 0xc0, 0x87, 0x32, 0x72, 0xab, 0x5e, 0x9c, 
    0x96, 0x12, 0x85, 0x4e, 0x47, 0x0d, 0xfc, 0xe2
};

static void calculate_block_cipher(uint8_t *p_cleartext, uint8_t *p_key) {
    // AES ECB構造体、ブロック暗号格納領域を初期化
    memset(&m_ecb_data, 0, sizeof(nrf_ecb_hal_data_t));
    memset(block_cipher, 0, sizeof(block_cipher));

    // 引数のcleartext, keyからブロック暗号(16バイト)を自動生成
    memcpy(m_ecb_data.key, p_key, SOC_ECB_KEY_LENGTH);
    memcpy(m_ecb_data.cleartext, p_cleartext, SOC_ECB_CLEARTEXT_LENGTH);
    sd_ecb_block_encrypt(&m_ecb_data);

    // ブロック暗号を、引数の領域にセット
    memcpy(block_cipher, m_ecb_data.ciphertext, SOC_ECB_CIPHERTEXT_LENGTH);

    // 生成されたブロック暗号をdebug
    NRF_LOG_DEBUG("block_cipher \r\n");
    dump_octets(block_cipher, SOC_ECB_CIPHERTEXT_LENGTH);
}

void ble_u2f_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet) {
    // 最初のブロック暗号生成時の入力には
    // 初期化ベクターを指定
    uint8_t *cleartext = m_initialization_vector;

    for (int i = 0; i < packet_length; i += ECB_BLOCK_LENGTH) {
        // ブロック暗号を生成して暗号化
        calculate_block_cipher(cleartext, m_ecb_key);
        for (int j = 0; j < ECB_BLOCK_LENGTH; j++) {
            out_packet[i + j] = packet[i + j] ^ block_cipher[j];
        }
        // 次回ブロック暗号生成時に入力となる領域を設定
        cleartext = out_packet + i;
    }

    // 暗号化前後のバイト配列(64バイト)をdebug
    NRF_LOG_DEBUG("non encrypted array \r\n");
    dump_octets(packet, packet_length);
    NRF_LOG_DEBUG("encrypted array \r\n");
    dump_octets(out_packet, packet_length);
}

void ble_u2f_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet) {
    // 最初のブロック暗号生成時の入力には
    // 初期化ベクターを指定
    uint8_t *cleartext = m_initialization_vector;

    for (int i = 0; i < packet_length; i += ECB_BLOCK_LENGTH) {
        // ブロック暗号を生成して暗号化
        calculate_block_cipher(cleartext, m_ecb_key);
        for (int j = 0; j < ECB_BLOCK_LENGTH; j++) {
            out_packet[i + j] = packet[i + j] ^ block_cipher[j];
        }
        // 次回ブロック暗号生成時に入力となる領域を設定
        cleartext = packet + i;
    }

    // 復号化後のバイト配列(64バイト)をdebug
    NRF_LOG_DEBUG("decrypted array \r\n");
    dump_octets(out_packet, packet_length);
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
