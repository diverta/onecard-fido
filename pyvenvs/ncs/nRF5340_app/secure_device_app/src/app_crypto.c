/* 
 * File:   app_crypto.c
 * Author: makmorit
 *
 * Created on 2021/05/12, 9:59
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_crypto);

#define LOG_DEBUG_AES_PLAINTEXT_DATA    false
#define LOG_DEBUG_AES_ENCRYPTED_DATA    false
#define LOG_DEBUG_AES_DECRYPTED_DATA    false

#define AES_KEY_SIZE 32

// 作業領域
static uint8_t m_aes_iv[16];

static bool aes_cbc_256_init(mbedtls_cipher_context_t *p_ctx)
{
    mbedtls_cipher_init(p_ctx);
    const mbedtls_cipher_info_t *p_cipher_info = mbedtls_cipher_info_from_values(MBEDTLS_CIPHER_ID_AES, AES_KEY_SIZE * 8, MBEDTLS_MODE_CBC);
    if (p_cipher_info == NULL) {
        LOG_ERR("mbedtls_cipher_info_from_values returns NULL");
        return false;
    }

    int ret = mbedtls_cipher_setup(p_ctx, p_cipher_info);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_setup returns %d", ret);
        return false;
    }

    return true;
}

static bool aes_cbc_256_prepare(mbedtls_cipher_context_t *p_ctx, uint8_t *p_key, mbedtls_operation_t operation, mbedtls_cipher_padding_t padding)
{
    int ret = mbedtls_cipher_setkey(p_ctx, p_key, AES_KEY_SIZE * 8, operation);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_setkey returns %d", ret);
        return false;
    }

    ret = mbedtls_cipher_set_padding_mode(p_ctx, padding);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_set_padding_mode returns %d", ret);
        return false;
    }

    return true;
}

bool app_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted, size_t *encrypted_size) 
{
#if LOG_DEBUG_ENCRYPTED_DATA
    LOG_DBG("%d bytes", plaintext_size);
    LOG_HEXDUMP_DBG(p_plaintext, plaintext_size, "Plaintext data");
#endif

    mbedtls_cipher_context_t ctx;
    if (aes_cbc_256_init(&ctx) == false) {
        return false;
    }
    if (aes_cbc_256_prepare(&ctx, p_key, MBEDTLS_ENCRYPT, MBEDTLS_PADDING_NONE) == false) {
        return false;
    }

    memset(m_aes_iv, 0, sizeof(m_aes_iv));
    int ret = mbedtls_cipher_crypt(&ctx, m_aes_iv, sizeof(m_aes_iv), p_plaintext, plaintext_size, encrypted, encrypted_size);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_crypt returns %d", ret);
        return false;
    }

    return true;
}

bool app_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted, size_t *decrypted_size) 
{
#if LOG_DEBUG_ENCRYPTED_DATA
    LOG_DBG("%d bytes", encrypted_size);
    LOG_HEXDUMP_DBG(p_encrypted, encrypted_size, "Encrypted data");
#endif

    mbedtls_cipher_context_t ctx;
    if (aes_cbc_256_init(&ctx) == false) {
        return false;
    }
    if (aes_cbc_256_prepare(&ctx, p_key, MBEDTLS_DECRYPT, MBEDTLS_PADDING_NONE) == false) {
        return false;
    }

    memset(m_aes_iv, 0, sizeof(m_aes_iv));
    int ret = mbedtls_cipher_crypt(&ctx, m_aes_iv, sizeof(m_aes_iv), p_encrypted, encrypted_size, decrypted, decrypted_size);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_crypt returns %d", ret);
        return false;
    }

#if LOG_DEBUG_DECRYPTED_DATA
    LOG_DBG("%d bytes", *decrypted_size);
    LOG_HEXDUMP_DBG(decrypted, *decrypted_size, "Decrypted data");
#endif

    return true;
}
