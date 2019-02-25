/* 
 * File:   ctap2_client_pin_crypto.c
 * Author: makmorit
 *
 * Created on 2019/02/25, 15:11
 */
#include "sdk_common.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "fido_crypto_ecb.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// AESで使用する作業用エリア
#define ECB_BLOCK_LENGTH 16
static uint8_t  m_block_cipher[ECB_BLOCK_LENGTH];
static uint8_t  m_initialization_vector[ECB_BLOCK_LENGTH];
static uint8_t *m_password;

static nrf_ecb_hal_data_t m_ecb_data;

// AES CBCモード
#define AES_CBC_MODE_ENCRYPTION 0
#define AES_CBC_MODE_DECRYPTION 1

bool ctap2_client_pin_crypto_init(uint8_t *p_password)
{
    // 暗号が格納されている領域の先頭アドレスを保持
    m_password = p_password;

    // 初期化ベクターは0とする
    memset(m_initialization_vector, 0, ECB_BLOCK_LENGTH);
    return true;
}

static void calculate_block_cipher(uint8_t *p_cleartext, uint8_t *p_key) 
{
    // AES構造体、ブロック暗号格納領域を初期化
    memset(&m_ecb_data, 0, sizeof(nrf_ecb_hal_data_t));
    memset(m_block_cipher, 0, sizeof(m_block_cipher));

    // 引数のcleartext, keyから16バイト分の暗号／復号計算
    memcpy(m_ecb_data.key, p_key, SOC_ECB_KEY_LENGTH);
    memcpy(m_ecb_data.cleartext, p_cleartext, SOC_ECB_CLEARTEXT_LENGTH);
    uint32_t err_code = sd_ecb_block_encrypt(&m_ecb_data);
    APP_ERROR_CHECK(err_code);

    // 計算結果を、引数の領域にセット
    memcpy(m_block_cipher, m_ecb_data.ciphertext, SOC_ECB_CIPHERTEXT_LENGTH);
}

void ctap2_client_pin_decrypt(uint8_t *p_ciphertext, size_t ciphertext_size, uint8_t *out_packet) 
{
    uint8_t *ciphertext;
    uint8_t *ciphertext_prev = m_initialization_vector;
    
    for (int i = 0; i < ciphertext_size; i += ECB_BLOCK_LENGTH) {
        // ブロックごとに復号化
        ciphertext = p_ciphertext + i;
        calculate_block_cipher(ciphertext, m_password);
        // 前ブロックの暗号化データと、復号化されたブロックデータのXORを計算
        for (int j = 0; j < ECB_BLOCK_LENGTH; j++) {
            out_packet[i + j] = m_block_cipher[j] ^ ciphertext_prev[j];
        }
        ciphertext_prev = ciphertext;
    }
}
